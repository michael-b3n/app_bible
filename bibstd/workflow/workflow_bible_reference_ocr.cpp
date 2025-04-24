#include "workflow/workflow_bible_reference_ocr.hpp"
#include "bible/reference_range.hpp"
#include "core/core_bible_reference.hpp"
#include "core/core_bible_reference_ocr.hpp"
#include "core/core_bibleserver_lookup.hpp"
#include "core/core_settings.hpp"
#include "core/core_tesseract.hpp"
#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "data/save_as_bitmap.hpp"
#include "system/filesystem.hpp"
#include "system/screen.hpp"
#include "txt/chars.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/date.hpp"
#include "util/format.hpp"

#include <array>
#include <numeric>
#include <thread>

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_bible_reference_ocr_settings::workflow_bible_reference_ocr_settings()
  : app_framework::settings_base{"OCR"}
  , translations{core_settings_->create_setting("ocr.translations", "Translations", std::vector<bible::translation>{bible::translation::ngu, bible::translation::elb})}
// clang-format on
{
}

///
///
workflow_bible_reference_ocr::workflow_bible_reference_ocr(language language)
  : core_bible_reference_ocr_{std::make_unique<core::core_bible_reference_ocr>()}
  , core_bible_reference_{std::make_unique<core::core_bible_reference>()}
  , core_bibleserver_lookup_{std::make_unique<core::core_bibleserver_lookup>()}
  , core_tesseract_{std::make_unique<core::core_tesseract>(language)}
{
}

///
///
workflow_bible_reference_ocr::~workflow_bible_reference_ocr() noexcept = default;

///
///
auto workflow_bible_reference_ocr::run_once(const settings_type& settings) -> void
{
  const auto cursor_position = system::screen::cursor_position();
  app_framework::thread_pool::queue_task(
    [this, settings, cursor_position]()
    {
      settings_ = settings;
      data_.current_cursor_position = cursor_position;
      find_references();
    },
    strand_id_,
    app_framework::thread_pool::queue_rule::overwrite
  );
}

///
///
auto workflow_bible_reference_ocr::find_references() -> void
{
  if(!set_capture_areas())
  {
    LOG_WARN("failed to define capture areas: cursor_position={}", data_.current_cursor_position);
    return;
  }

  std::vector<bible::reference_range> references{};
  std::ranges::any_of(
    data_.capture_areas,
    [&](const auto& screen_area)
    {
      if(!capture_img(screen_area) || !core_tesseract_->recognize(std::nullopt))
      {
        LOG_WARN("capture screen failed: screen_area={}", screen_area);
        return false;
      }

      const auto image_dimensions = screen_rect_type({0, 0}, screen_area.horizontal_range(), screen_area.vertical_range());
      const auto relative_cursor_pos = data_.current_cursor_position - screen_area.origin();

      LOG_DEBUG(
        "start OCR: screen_area={}, cursor_position={}, image_dimensions={}, relative_cursor_pos={}",
        screen_area,
        data_.current_cursor_position,
        image_dimensions,
        relative_cursor_pos
      );

      auto references_optional = parse_tesseract_recognition(image_dimensions, relative_cursor_pos);
      references = references_optional.value_or(decltype(references){});
      return references_optional.has_value();
    }
  );
  LOG_INFO("OCR reference search finished: found=[{}]", util::format::join(references, ", "));
  std::ranges::for_each(
    references,
    [&](const auto& reference_range) { core_bibleserver_lookup_->open(reference_range, settings_->translations->value()); }
  );
}

///
///
auto workflow_bible_reference_ocr::set_capture_areas() -> bool
{
  data_.capture_areas.clear();
  const auto window_rect = system::screen::window_at(data_.current_cursor_position);
  if(!window_rect)
  {
    return false;
  }
  const auto height = data_.current_char_height.value_or(window_rect->vertical_range() / vertical_range_denominator) * 2;
  const auto width = height * height_to_width_ratio;
  const auto valid = std::ranges::all_of(
    capture_ocr_area_steps,
    [&](const auto i)
    {
      const auto x_origin = data_.current_cursor_position.x() - (i * width / 2);
      const auto y_origin = data_.current_cursor_position.y() - (i * height / 2);
      const auto rect = screen_rect_type::overlap(*window_rect, screen_rect_type({x_origin, y_origin}, i * width, i * height));
      const auto valid_rect = rect.has_value();
      if(valid_rect)
      {
        data_.capture_areas.push_back(*rect);
      }
      return valid_rect;
    }
  );
  valid ? data_.capture_areas.push_back(*window_rect) : data_.capture_areas.clear();
  return valid;
}

///
///
auto workflow_bible_reference_ocr::capture_img(const screen_rect_type& rect) -> bool
{
  auto success = false;
  core_tesseract_->set_image(
    [&](auto& pixel_plane)
    {
      pixel_plane.width = rect.horizontal_range();
      pixel_plane.height = rect.vertical_range();
      if(pixel_plane.data.size() < pixel_plane.width * pixel_plane.height)
      {
        pixel_plane.data.resize(pixel_plane.width * pixel_plane.height);
      }
      success = system::screen::capture(rect, pixel_plane);
    }
  );
  return success;
}

///
///
auto workflow_bible_reference_ocr::parse_tesseract_recognition(
  const screen_rect_type& image_dimensions, const screen_coordinates_type& relative_cursor_pos
) -> std::optional<std::vector<bible::reference_range>>
{
  auto references = std::vector<bible::reference_range>{};
  auto valid_capture_area = false;

  core_tesseract_->for_each_while(
    core::core_tesseract::text_resolution::paragraph,
    [&](const auto text_paragraph, const auto& bounding_box)
    {
      LOG_DEBUG("parse paragraph: paragraph_bounding_box={}, relative_cursor_pos={}", bounding_box, relative_cursor_pos);
      const auto cursor_in_paragraph = std::decay_t<decltype(bounding_box)>::contains(bounding_box, relative_cursor_pos);
      if(cursor_in_paragraph)
      {
        const auto position_data = get_reference_position_main(relative_cursor_pos);
        if(position_data)
        {
          auto parse_result = core_bible_reference_->parse(position_data->text, position_data->index);
          // Parse result might be empty but the capture area is still valid.
          // This is the case when the text is not a valid reference but the characters found in the image are
          // within the capture area including a safety margin. In this case no larger area is captured.
          valid_capture_area = is_valid_capture_area(image_dimensions, *position_data, parse_result.index_range_origin);

          // If the capture area is valid but no references are found, we parse other high confidence OCR choices.
          // If a parse result is found and the area is valid we break out.
          if(valid_capture_area && parse_result.ranges.empty())
          {
            const auto position_data_choices = get_reference_position_choices(relative_cursor_pos);
            std::ranges::any_of(
              position_data_choices,
              [&](const auto& position_data_choice)
              {
                parse_result = core_bible_reference_->parse(position_data_choice.text, position_data_choice.index);
                valid_capture_area =
                  is_valid_capture_area(image_dimensions, position_data_choice, parse_result.index_range_origin);
                return valid_capture_area && !parse_result.ranges.empty();
              }
            );
          }

          references = parse_result.ranges;
        }
      }
      return !cursor_in_paragraph;
    }
  );
  LOG_DEBUG(
    "parse recognition result: references=[{}], image_dimensions={}, relative_cursor_pos={}",
    util::format::join(references, ", "),
    image_dimensions,
    relative_cursor_pos
  );
  return valid_capture_area ? std::make_optional(references) : std::nullopt;
}

///
///
auto workflow_bible_reference_ocr::get_reference_position_main(const screen_coordinates_type& relative_cursor_pos)
  -> std::optional<reference_position_data>
{
  auto text = std::string{};
  std::vector<character_data> char_data{};

  core_tesseract_->for_each(
    core::core_tesseract::text_resolution::character,
    [&](const auto text_character, const auto& bounding_box)
    {
      text.append(text_character.data(), text_character.size());
      const auto abs_distance = std::abs(screen_coordinates_type::distance(bounding_box.center(), relative_cursor_pos));
      char_data.insert(char_data.cend(), text_character.size(), {abs_distance, bounding_box});
    }
  );
  const auto distance_index = get_min_distance_index(char_data);
  auto result = std::optional<reference_position_data>{};
  if(distance_index)
  {
    result = reference_position_data{std::move(text), std::move(char_data), *distance_index};
  }
  LOG_DEBUG(
    "reference position main result: [{}], relative_cursor_pos={}",
    result ? std::format("text=\"{}\", index={}", result->text, result->index) : std::string{"none"},
    relative_cursor_pos
  );
  return result;
}

///
///
auto workflow_bible_reference_ocr::get_reference_position_choices(const screen_coordinates_type& relative_cursor_pos)
  -> std::vector<reference_position_data>
{
  auto choices_list = std::vector<tesseract_choices>{};
  auto choices_char_data = std::vector<character_data>{};
  core_tesseract_->for_each_choices(
    [&](const auto& choices, const auto& bounding_box)
    {
      choices_list.emplace_back(choices);
      const auto abs_distance = std::abs(screen_coordinates_type::distance(bounding_box.center(), relative_cursor_pos));
      choices_char_data.emplace_back(abs_distance, bounding_box);
    }
  );

  const auto matching_indexed_strings = core_bible_reference_ocr_->match_choices_to_bible_book(
    choices_list,
    [&](const auto& choices)
    {
      const auto main_symbol_info = txt::chars::char_info(choices.front().symbol, 0);
      return main_symbol_info && (main_symbol_info->char_category == txt::chars::category::letter ||
                                  main_symbol_info->char_category == txt::chars::category::digit);
    }
  );

  assert(choices_list.size() == choices_char_data.size());
  assert(std::ranges::all_of(matching_indexed_strings, [&](const auto& e) { return e.size() == choices_list.size(); }));

  auto result = std::vector<reference_position_data>{};
  std::ranges::for_each(
    matching_indexed_strings,
    [&](const auto& indexed_strings)
    {
      auto text = std::string{};
      std::vector<character_data> char_data{};

      std::ranges::for_each(
        indexed_strings.indexed_chars(),
        [&](const auto& index_character_pair)
        {
          const auto [index, character] = index_character_pair;
          text.push_back(character);
          char_data.push_back(choices_char_data.at(index));
        }
      );

      const auto distance_index = get_min_distance_index(char_data);
      if(distance_index)
      {
        result.emplace_back(std::move(text), std::move(char_data), *distance_index);
      }
    }
  );

  const auto format_result = [&]
  {
    const auto result_range =
      result | std::views::transform([&](const auto& e) { return std::format("(text=\"{}\", index={})", e.text, e.index); });
    return util::format::join(result_range, ", ");
  };
  LOG_DEBUG("reference position choices result: [{}], relative_cursor_pos={}", format_result(), relative_cursor_pos);
  return result;
}

///
///
auto workflow_bible_reference_ocr::get_min_distance_index(const std::vector<character_data>& char_data)
  -> std::optional<std::size_t>
{
  auto result = std::optional<std::size_t>{};
  if(const auto min_element =
       std::ranges::min_element(char_data, [](const auto& a, const auto& b) { return a.distance < b.distance; });
     min_element != std::ranges::cend(char_data))
  {
    result = static_cast<std::size_t>(std::ranges::distance(char_data.cbegin(), min_element));
  }
  else
  {
    LOG_WARN("no minimum distance found in character data: char_data_size={}", char_data.size());
  }
  return result;
}

///
///
auto workflow_bible_reference_ocr::is_valid_capture_area(
  const screen_rect_type& image_dimensions, const reference_position_data& position_data, index_range_type index_range
) -> bool
{
  auto char_height = std::int32_t{0};
  const auto get_char_height = [&](const auto i)
  { char_height = std::max(char_height, position_data.char_data.at(i).bounding_box.vertical_range()); };
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, position_data.char_data.size()), [&](const auto& char_data) { get_char_height(char_data); }
  );

  const auto is_in_bounds = [&](const auto i) -> bool
  {
    const auto& char_bounding_box = position_data.char_data.at(i).bounding_box;
    const auto bounding_box_with_margin = screen_rect_type(
      char_bounding_box.origin() - screen_rect_type::coordinates_type(char_height, char_height / 4),
      char_bounding_box.horizontal_range() + (char_height * 2),
      char_bounding_box.vertical_range() + char_height
    );
    return screen_rect_type::contains(image_dimensions, bounding_box_with_margin);
  };
  auto result = false;
  if(index_range_type::empty(index_range))
  {
    result = std::ranges::any_of(
      std::views::iota(std::size_t{0}, position_data.char_data.size()), [&](const auto i) { return is_in_bounds(i); }
    );
  }
  else
  {
    result = std::ranges::all_of(
      std::views::iota(index_range.begin, index_range.end) |
        std::views::filter([&](const auto i) { return i < position_data.char_data.size(); }),
      [&](const auto i) { return is_in_bounds(i); }
    );
  }
  if(!result)
  {
    LOG_DEBUG("invalid capture area: image_dimensions={}, char_height={}", image_dimensions, char_height);
  }
  return result;
}

} // namespace bibstd::workflow
