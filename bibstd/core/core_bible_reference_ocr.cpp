#include "core/core_bible_reference_ocr.hpp"
#include "bible/book_name_variants_de.hpp"
#include "system/screen.hpp"
#include "util/log.hpp"
#include "util/string.hpp"

#include <algorithm>

namespace bibstd::core
{

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
auto core_bible_reference_ocr::generate_capture_areas(
  const screen_coordinates_type& cursor_position, const std::optional<std::uint32_t> char_height
) const -> std::vector<screen_rect_type>
{
  auto result = std::vector<screen_rect_type>{};
  const auto window_rect = system::screen::window_at(cursor_position);
  if(!window_rect)
  {
    return result;
  }
  const auto height = char_height.value_or(window_rect->vertical_range() / vertical_range_to_full_screen_factor) * 2;
  const auto width = height * height_to_width_ratio;
  const auto valid = std::ranges::all_of(
    capture_ocr_area_steps,
    [&](const auto i)
    {
      const auto x_origin = cursor_position.x() - (i * width / 2);
      const auto y_origin = cursor_position.y() - (i * height / 2);
      const auto rect = screen_rect_type::overlap(*window_rect, screen_rect_type({x_origin, y_origin}, i * width, i * height));
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
  const screen_rect_type& image_dimensions,
  const core_bible_reference_ocr_common::reference_position_data& position_data,
  const core_bible_reference_ocr_common::index_range_type& index_range
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
      char_bounding_box.origin() -
        screen_rect_type::coordinates_type(char_height, static_cast<std::int32_t>(char_height * vertical_margin_multiplier)),
      char_bounding_box.horizontal_range() + static_cast<std::int32_t>(char_height * horizontal_margin_multiplier),
      char_bounding_box.vertical_range() + char_height
    );
    return screen_rect_type::contains(image_dimensions, bounding_box_with_margin);
  };
  auto result = false;
  if(core_bible_reference_ocr_common::index_range_type::empty(index_range))
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

} // namespace bibstd::core
