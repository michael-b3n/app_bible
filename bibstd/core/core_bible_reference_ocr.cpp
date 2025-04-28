#include "core/core_bible_reference_ocr.hpp"
#include "bible/book_name_variants_de.hpp"
#include "core/core_tesseract.hpp"
#include "system/screen.hpp"
#include "txt/chars.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/format.hpp"
#include "util/log.hpp"
#include "util/string.hpp"

#include <algorithm>

namespace bibstd::core
{

///
///
core_bible_reference_ocr::core_bible_reference_ocr(core_tesseract_common::language language)
  : core_tesseract_{std::make_unique<core::core_tesseract>(language)}
{
}

///
///
core_bible_reference_ocr::~core_bible_reference_ocr() noexcept = default;

///
///
auto core_bible_reference_ocr::capture_and_recognize_area(const screen_rect_type& screen_area) const -> bool
{
  auto success = false;
  core_tesseract_->set_image(
    [&](auto& pixel_plane)
    {
      pixel_plane.width = screen_area.horizontal_range();
      pixel_plane.height = screen_area.vertical_range();
      if(pixel_plane.data.size() < pixel_plane.width * pixel_plane.height)
      {
        pixel_plane.data.resize(pixel_plane.width * pixel_plane.height);
      }
      success = system::screen::capture(screen_area, pixel_plane);
    }
  );
  if(success)
  {
    success = core_tesseract_->recognize(std::nullopt);
  }
  return success;
}

///
///
auto core_bible_reference_ocr::find_paragraph_bounding_box(const screen_coordinates_type& relative_cursor_position) const
  -> std::optional<screen_rect_type>
{
  auto result = std::optional<screen_rect_type>{};
  core_tesseract_->for_each_while(
    core::core_tesseract::text_resolution::paragraph,
    [&]([[maybe_unused]] const auto, const auto& paragraph_bounding_box)
    {
      const auto cursor_in_paragraph = screen_rect_type::contains(paragraph_bounding_box, relative_cursor_position);
      if(cursor_in_paragraph)
      {
        result = paragraph_bounding_box;
      }
      return !cursor_in_paragraph;
    }
  );
  return result;
}

///
///
auto core_bible_reference_ocr::find_main_reference_position_data(const screen_coordinates_type& relative_cursor_position) const
  -> std::optional<reference_position_data>
{
  auto text = std::string{};
  std::vector<character_data> char_data{};

  core_tesseract_->for_each(
    core::core_tesseract::text_resolution::character,
    [&](const auto text_character, const auto& bounding_box)
    {
      text.append(text_character.data(), text_character.size());
      const auto abs_distance = std::abs(screen_coordinates_type::distance(bounding_box.center(), relative_cursor_position));
      char_data.insert(char_data.cend(), text_character.size(), {abs_distance, bounding_box});
    }
  );
  const auto distance_index = min_distance_index(char_data);
  auto result = std::optional<reference_position_data>{};
  if(distance_index)
  {
    result = reference_position_data{std::move(text), std::move(char_data), *distance_index};
  }
  LOG_DEBUG(
    "main reference position result: [{}], relative_cursor_position={}",
    result ? std::format("text=\"{}\", index={}", result->text, result->cursor_character_index) : std::string{"none"},
    relative_cursor_position
  );
  return result;
}

///
///
auto core_bible_reference_ocr::find_reference_position_data_from_choices(
  const screen_coordinates_type& relative_cursor_position
) const -> std::vector<reference_position_data>
{
  auto choices_list = std::vector<tesseract_choices>{};
  auto choices_char_data = std::vector<character_data>{};
  core_tesseract_->for_each_choices(
    [&](const auto& choices, const auto& bounding_box)
    {
      choices_list.emplace_back(choices);
      const auto abs_distance = std::abs(screen_coordinates_type::distance(bounding_box.center(), relative_cursor_position));
      choices_char_data.emplace_back(abs_distance, bounding_box);
    }
  );
  const auto matching_indexed_strings = match_choices_to_bible_book(
    choices_list,
    [&](const auto& choices)
    {
      return txt::chars::is_char(choices.front().symbol, 0, txt::chars::category::letter) ||
             txt::chars::is_char(choices.front().symbol, 0, txt::chars::category::digit);
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
          text.push_back(index_character_pair.second);
          char_data.push_back(choices_char_data.at(index_character_pair.first));
        }
      );
      const auto distance_index = min_distance_index(char_data);
      if(distance_index)
      {
        result.emplace_back(std::move(text), std::move(char_data), *distance_index);
      }
    }
  );
  const auto format_result = [&]
  {
    const auto result_range =
      result |
      std::views::transform(
        [&](const auto& e) { return std::format("(text=\"{}\", cursor_character_index={})", e.text, e.cursor_character_index); }
      );
    return util::format::join(result_range, ", ");
  };
  LOG_DEBUG("reference position choices result: [{}], cursor_position={}", format_result(), relative_cursor_position);
  return result;
}

///
///
auto core_bible_reference_ocr::generate_capture_areas(
  const screen_coordinates_type& cursor_position, const std::uint16_t assumed_char_height
) const -> std::vector<screen_rect_type>
{
  auto result = std::vector<screen_rect_type>{};
  const auto window_rect = system::screen::window_at(cursor_position);
  if(!window_rect)
  {
    return result;
  }
  // The capture areas are defined dependent on the char height using a char_height_multiplier
  // and the height_to_width_ratio. A capture area step factor is used to scale the area.
  // The area is generated around the cursor position. The cursor position will be horizontally
  // and vertically in the middle.
  const auto height = static_cast<std::int32_t>(char_height_multiplier * assumed_char_height);
  const auto width = static_cast<std::int32_t>(height * height_to_width_ratio);
  const auto valid = std::ranges::all_of(
    capture_ocr_area_steps,
    [&](const auto i)
    {
      const auto half_width = static_cast<std::int32_t>(i * width / 2);
      const auto half_height = static_cast<std::int32_t>(i * height / 2);
      const auto x_origin = cursor_position.x() - half_width;
      const auto y_origin = cursor_position.y() - 1 * half_height; // origin is on top left
      const auto rect =
        screen_rect_type::overlap(*window_rect, screen_rect_type({x_origin, y_origin}, 2 * half_width, 2 * half_height));
      const auto valid_rect = rect.has_value();
      if(valid_rect)
      {
        result.push_back(*rect);
      }
      return valid_rect;
    }
  );
  valid ? result.push_back(*window_rect) : result.clear();
  return result;
}

///
///
auto core_bible_reference_ocr::is_valid_capture_area(
  const screen_coordinates_type& relative_cursor_position,
  const screen_rect_type& image_dimensions,
  const screen_rect_type& paragraph_dimensions,
  const reference_position_data& position_data,
  const core_bible_reference_ocr_common::index_range_type& index_range
) -> capture_area_validity_check_result
{
  auto result = capture_area_validity_check_result{};
  const auto line_position_data = find_line_position_data(relative_cursor_position);
  if(!line_position_data)
  {
    return result;
  }
  const auto top = [](const auto& box) { return box.origin().y() + box.vertical_range(); };
  const auto bottom = [](const auto& box) { return box.origin().y(); };
  const auto left = [](const auto& box) { return box.origin().x(); };
  const auto right = [](const auto& box) { return box.origin().x() + box.horizontal_range(); };

  const auto char_height = line_position_data->line_bounding_boxes.at(line_position_data->cursor_line_index).vertical_range();
  const auto vertical_margin = static_cast<std::int32_t>(char_height * vertical_margin_multiplier / 2);
  const auto horizontal_margin = static_cast<std::int32_t>(char_height * horizontal_margin_multiplier / 2);
  result.detected_char_height = boost::numeric_cast<std::uint16_t>(char_height);

  const auto prev_and_next_lines_within_bounds = [&]
  {
    const auto line_index = line_position_data->cursor_line_index;
    return line_index > 0 && line_index + 1 < line_position_data->line_bounding_boxes.size() &&
           top(line_position_data->line_bounding_boxes.at(line_index - 1)) + vertical_margin < top(image_dimensions) &&
           bottom(line_position_data->line_bounding_boxes.at(line_index + 1)) > bottom(image_dimensions) + vertical_margin;
  };

  if(core_bible_reference_ocr_common::index_range_type::empty(index_range) || position_data.char_data.empty())
  {
    result.valid = prev_and_next_lines_within_bounds() &&
                   (left(image_dimensions) + horizontal_margin) < left(paragraph_dimensions) &&
                   (right(paragraph_dimensions) + horizontal_margin) < right(image_dimensions);
  }
  else
  {
    // If the found reference range is well within the reference borders, it is assumed,
    // that it will not continue to the next line. If not, the image border must include
    // the left paragraph border as well and there must be a complete text line above and
    // below the line where the cursor is contained.
    const auto ends_before_paragraph_border = std::ranges::all_of(
      std::views::iota(index_range.begin, index_range.end) |
        std::views::filter([&](const auto i) { return i < position_data.char_data.size(); }),
      [&](const auto i)
      {
        const auto& char_bounding_box = position_data.char_data.at(i).bounding_box;
        const auto char_right_border = char_bounding_box.origin().x() + char_bounding_box.horizontal_range();
        return char_right_border + 2 * horizontal_margin < right(paragraph_dimensions);
      }
    );
    auto valid_paragraph_area = ends_before_paragraph_border;
    if(!ends_before_paragraph_border)
    {
      const auto line_index = line_position_data->cursor_line_index;
      valid_paragraph_area =
        image_dimensions.origin().x() < left(paragraph_dimensions) && line_index > 0 &&
        line_index + 1 < line_position_data->line_bounding_boxes.size() &&
        top(line_position_data->line_bounding_boxes.at(line_index - 1)) + vertical_margin < top(image_dimensions) &&
        bottom(line_position_data->line_bounding_boxes.at(line_index + 1)) > bottom(image_dimensions) + vertical_margin;
    }
    const auto valid_character_positions = std::ranges::all_of(
      std::views::iota(index_range.begin, index_range.end) |
        std::views::filter([&](const auto i) { return i < position_data.char_data.size(); }),
      [&](const auto i)
      {
        const auto& char_bounding_box = position_data.char_data.at(i).bounding_box;
        const auto bounding_box_with_margin = screen_rect_type(
          char_bounding_box.origin() - screen_rect_type::coordinates_type(horizontal_margin, vertical_margin),
          char_bounding_box.horizontal_range() + horizontal_margin * 2,
          char_bounding_box.vertical_range() + vertical_margin * 2
        );
        return screen_rect_type::contains(image_dimensions, bounding_box_with_margin);
      }
    );
    result.valid = valid_paragraph_area && valid_character_positions;
  }
  if(!result.valid)
  {
    LOG_DEBUG("invalid capture area: image_dimensions={}, char_height={}", image_dimensions, char_height);
  }
  return result;
}

///
///
auto core_bible_reference_ocr::match_choices_to_bible_book(
  const std::vector<tesseract_choices>& choices_list, const std::function<bool(const tesseract_choices&)>& choices_filter
) const -> std::vector<txt::indexed_strings>
{
  auto results = std::vector<txt::indexed_strings>{};
  std::ranges::for_each(
    bible::book_name_variants_de::name_variants_list,
    [&](const auto& element)
    {
      const auto& [_, name_variant] = element;
      const auto element_result = match_choices_to_string(choices_list, name_variant, choices_filter);
      results.insert(results.cend(), element_result.cbegin(), element_result.cend());
    }
  );
  return results;
}

///
///
auto core_bible_reference_ocr::match_choices_to_string(
  const std::vector<tesseract_choices>& choices_list,
  const std::string_view text_template,
  const std::function<bool(const tesseract_choices&)>& choices_filter
) const -> std::vector<txt::indexed_strings>
{
  auto results = std::vector<txt::indexed_strings>{};
  if(!text_template.empty())
  {
    const auto indexed_strings = [&]
    {
      auto result = txt::indexed_strings{};
      std::ranges::for_each(choices_list, [&](const auto& choices) mutable { result.append_string(choices.front().symbol); });
      return result;
    }();

    std::ranges::for_each(
      std::views::iota(decltype(choices_list.size()){0}, choices_list.size()),
      [&](const auto choices_list_offset) mutable
      {
        auto text_template_found = false;
        auto local_indexed_strings = indexed_strings;
        std::ranges::any_of(
          std::views::iota(choices_list_offset, choices_list.size()) |
            std::views::filter([&](const auto index) { return choices_filter(choices_list.at(index)); }),
          [&, text_template_position = std::size_t{0}](const auto choices_list_index) mutable
          {
            const auto& choices = choices_list.at(choices_list_index);
            const auto choice_optional = find_chars_begin_match(choices, text_template.substr(text_template_position));
            if(choice_optional)
            {
              text_template_position += choice_optional->symbol.size();
              local_indexed_strings.overwrite_at(choices_list_index, choice_optional->symbol);

              text_template_found = text_template_position >= text_template.size();
              if(text_template_found)
              {
                text_template_position = 0;
              }
              return text_template_found;
            }
            return true;
          }
        );
        if(text_template_found)
        {
          results.emplace_back(std::move(local_indexed_strings));
        }
      }
    );
  }
  return results;
}

///
///
auto core_bible_reference_ocr::find_chars_begin_match(const tesseract_choices& choices, const std::string_view chars) const
  -> std::optional<tesseract_choice>
{
  auto result = std::optional<tesseract_choice>{};
  std::ranges::for_each(
    choices,
    [&](const auto& choice)
    {
      auto match_found = util::starts_with(chars, choice.symbol);
      if(match_found)
      {
        result = choice;
      }
    }
  );
  return result;
}

///
///
auto core_bible_reference_ocr::min_distance_index(const std::vector<character_data>& char_data) const
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
auto core_bible_reference_ocr::find_line_position_data(const screen_coordinates_type& relative_cursor_position) const
  -> std::optional<line_position_data>
{
  auto line_bounding_boxes = decltype(line_position_data::line_bounding_boxes){};
  auto cursor_line_index = std::optional<std::size_t>{};
  core_tesseract_->for_each(
    core::core_tesseract::text_resolution::line,
    [&]([[maybe_unused]] const auto, const auto& line_bounding_box)
    {
      line_bounding_boxes.push_back(line_bounding_box);
      const auto contains_cursor = screen_rect_type::contains(line_bounding_box, relative_cursor_position);
      if(contains_cursor)
      {
        cursor_line_index = line_bounding_boxes.size() - 1;
      }
    }
  );
  return cursor_line_index ? std::make_optional(line_position_data{std::move(line_bounding_boxes), *cursor_line_index})
                           : std::nullopt;
}

} // namespace bibstd::core
