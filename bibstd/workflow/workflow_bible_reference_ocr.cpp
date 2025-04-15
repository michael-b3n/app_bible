#include "workflow/workflow_bible_reference_ocr.hpp"
#include "bible/reference_range.hpp"
#include "core/core_bible_reference.hpp"
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
  : core_bible_reference_{std::make_unique<core::core_bible_reference>()}
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
    LOG_WARN(log_channel, "Failed to define capture areas: cursor_position={}", data_.current_cursor_position);
    return;
  }

  auto references = std::vector<bible::reference_range>{};
  std::ranges::any_of(
    data_.capture_areas,
    [&](const auto& screen_area)
    {
      if(!capture_img(screen_area) || !core_tesseract_->recognize(std::nullopt))
      {
        LOG_WARN(log_channel, "Capture screen failed: screen_area={}", screen_area);
        return false;
      }

      const auto image_dimensions = bounding_box_type({0, 0}, screen_area.horizontal_range(), screen_area.vertical_range());
      const auto relative_cursor_position = data_.current_cursor_position - screen_area.origin();

      LOG_DEBUG(
        log_channel,
        "Start OCR: screen_area={}, cursor_position={}, image_dimensions={}, relative_cursor_position={}",
        screen_area,
        data_.current_cursor_position,
        image_dimensions,
        relative_cursor_position
      );

      auto found_references = false;
      core_tesseract_->for_each_until(
        core::core_tesseract::text_resolution::paragraph,
        [&](const auto text_paragraph, const auto& bounding_box)
        {
          const auto found = std::decay_t<decltype(bounding_box)>::contains(bounding_box, relative_cursor_position);
          if(found)
          {
            LOG_DEBUG(
              log_channel,
              "Cursor found in paragraph: bounding_box={}, relative_cursor_position={}",
              bounding_box,
              relative_cursor_position
            );
            const auto position_data = get_reference_position_data(relative_cursor_position, text_paragraph, bounding_box);
            found_references = position_data.has_value();
            if(position_data)
            {
              LOG_DEBUG(log_channel, "Position data found: index={}, text=\"{}\"", position_data->index, position_data->text);
              const auto parse_result = core_bible_reference_->parse(position_data->text, position_data->index);
              LOG_DEBUG(
                log_channel,
                "Parse result: index_range_origin={}, references=[{}]",
                parse_result.index_range_origin,
                util::format::join(parse_result.ranges, ", ")
              );
              const auto valid_capture_area =
                is_valid_capture_area(image_dimensions, parse_result.index_range_origin, *position_data);
              found_references = valid_capture_area && !parse_result.ranges.empty();
              if(found_references)
              {
                references = parse_result.ranges;
              }
            }
            else
            {
              LOG_DEBUG(log_channel, "No position data found in paragraph: found_references={}", found_references);
            }
          }
          else
          {
            LOG_DEBUG(
              log_channel,
              "Skip paragraph ocr since no cursor: bounding_box={}, relative_cursor_position={}",
              bounding_box,
              relative_cursor_position
            );
          }
          return found;
        }
      );
      return found_references;
    }
  );
  LOG_INFO(log_channel, "OCR reference search finished: found=[{}]", util::format::join(references, ", "));
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
auto workflow_bible_reference_ocr::get_reference_position_data(
  const screen_coordinates_type& relative_cursor_position,
  const std::string_view text_paragraph,
  const bounding_box_type& bounding_box
) -> std::optional<reference_position_data>
{
  auto text = std::string{};
  text.reserve(text_paragraph.size());
  decltype(reference_position_data::char_data) char_data{};
  char_data.reserve(text_paragraph.size());

  core_tesseract_->for_each(
    core::core_tesseract::text_resolution::character,
    [&](const auto text_character, const auto& bounding_box)
    {
      text.append(text_character.data(), text_character.size());
      const auto abs_distance = std::abs(screen_coordinates_type::distance(bounding_box.center(), relative_cursor_position));
      char_data.insert(char_data.cend(), text_character.size(), {abs_distance, bounding_box});
    }
  );

  auto result = std::optional<reference_position_data>{};
  if(const auto min_element =
       std::ranges::min_element(char_data, [](const auto& a, const auto& b) { return a.distance < b.distance; });
     min_element != std::ranges::cend(char_data))
  {
    const auto index = std::ranges::distance(char_data.cbegin(), min_element);
    result = reference_position_data{std::move(text), std::move(char_data), boost::numeric_cast<std::size_t>(index)};
  }
  return result;
}

///
///
auto workflow_bible_reference_ocr::is_valid_capture_area(
  const bounding_box_type& image_dimensions, index_range_type index_range, const reference_position_data& reference_position
) -> bool
{
  auto char_width = std::int32_t{0};
  const auto is_digit = [&](const auto i) -> bool
  {
    const auto char_info = txt::chars::char_info(reference_position.text, i);
    return char_info && char_info->char_category == txt::chars::category::digit;
  };
  std::ranges::for_each(
    std::views::iota(index_range.begin, index_range.end) |
      std::views::filter([&](const auto i) { return i < reference_position.char_data.size(); }) | std::views::filter(is_digit),
    [&](const auto i) { char_width = std::max(char_width, reference_position.char_data.at(i).bounding_box.horizontal_range()); }
  );
  const auto safety_margin_side = char_width * 2;
  const auto is_in_bounds = [&](const auto i) -> bool
  {
    const auto& char_bounding_box = reference_position.char_data.at(i).bounding_box;
    const auto bounding_box_with_margin = bounding_box_type(
      char_bounding_box.origin() - bounding_box_type::coordinates_type(safety_margin_side, 0),
      char_bounding_box.horizontal_range() + (safety_margin_side * 2),
      char_bounding_box.vertical_range()
    );

    const auto contains = bounding_box_type::contains(image_dimensions, bounding_box_with_margin);
    if(!contains)
    {
      LOG_DEBUG(
        log_channel,
        "Invalid capture area: char_bounding_box={}, char_margin_bounding_box={}, image_dimensions={}",
        char_bounding_box,
        bounding_box_with_margin,
        image_dimensions
      );
    }
    return contains;
  };
  return std::ranges::all_of(
    std::views::iota(index_range.begin, index_range.end) |
      std::views::filter([&](const auto i) { return i < reference_position.char_data.size(); }),
    [&](const auto i) { return is_in_bounds(i); }
  );
}

} // namespace bibstd::workflow
