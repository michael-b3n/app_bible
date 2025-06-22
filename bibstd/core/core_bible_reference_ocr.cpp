#include "core/core_bible_reference_ocr.hpp"
#include "bible/book_name_variants_de.hpp"
#include "core/core_tesseract.hpp"
#include "system/screen.hpp"
#include "txt/chars.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/format.hpp"
#include "util/log.hpp"
#include "util/string.hpp"
#include "util/timer.hpp"

#include <algorithm>

namespace bibstd::core
{
namespace detail
{

///
/// Get the top position of a rectangle.
/// The top position is defined as the y coordinate of the origin plus the vertical range.
/// \param rect Rectangle for which the top position is calculated
/// \return top position of the rectangle
///
auto top(const util::screen_types::screen_rect_type& rect) -> std::int32_t
{
  return rect.origin().y() + rect.vertical_range();
}

///
/// Get the bottom position of a rectangle.
/// The bottom position is defined as the y coordinate of the origin.
/// \param rect Rectangle for which the bottom position is calculated
/// \return bottom position of the rectangle
///
auto bottom(const util::screen_types::screen_rect_type& rect) -> std::int32_t
{
  return rect.origin().y();
}

///
/// Get the left position of a rectangle.
/// The left position is defined as the x coordinate of the origin.
/// \param rect Rectangle for which the left position is calculated
/// \return left position of the rectangle
///
auto left(const util::screen_types::screen_rect_type& rect) -> std::int32_t
{
  return rect.origin().x();
}

///
/// Get the right position of a rectangle.
/// The right position is defined as the x coordinate of the origin plus the horizontal range.
/// \param rect Rectangle for which the right position is calculated
/// \return right position of the rectangle
///
auto right(const util::screen_types::screen_rect_type& rect) -> std::int32_t
{
  return rect.origin().x() + rect.horizontal_range();
}

} // namespace detail

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
auto core_bible_reference_ocr::capture_ocr_image(const screen_coordinates_type& cursor_position) const
  -> std::optional<screen_coordinates_type>
{
  SCOPED_TIMER_LOG();
  const auto window_rect = system::screen::window_at(cursor_position);
  if(!window_rect)
  {
    return std::nullopt;
  }
  auto pixel_plane = pixel_plane_type{};
  auto success = system::screen::capture(*window_rect, pixel_plane);
  if(success)
  {
    core_tesseract_->set_image(std::move(pixel_plane));
  }
  return cursor_position - window_rect->origin();
}

///
///
auto core_bible_reference_ocr::recognize_bounding_box(const screen_coordinates_type& relative_cursor_position) const
  -> std::optional<recognize_bounding_box_result>
{
  SCOPED_TIMER_LOG();
  const auto paragraph_bounding_boxes = core_tesseract_->bounding_boxes(core::core_tesseract::text_resolution::paragraph);
  const auto paragraph_bounding_box_iter = std::ranges::find_if(
    paragraph_bounding_boxes, [&](const auto& rect) { return screen_rect_type::contains(rect, relative_cursor_position); }
  );
  if(paragraph_bounding_box_iter == std::ranges::cend(paragraph_bounding_boxes))
  {
    return std::nullopt;
  }
  const auto line_bounding_boxes = core_tesseract_->bounding_boxes(core::core_tesseract::text_resolution::line);
  const auto iter = std::ranges::find_if(
    line_bounding_boxes, [&](const auto& rect) { return screen_rect_type::contains(rect, relative_cursor_position); }
  );
  if(iter != std::ranges::cend(line_bounding_boxes))
  {
    const auto top_line = [&]
    {
      auto top_line_view =
        line_bounding_boxes |
        std::views::filter([&](const auto& r) { return r.vertical_range() > 0 && detail::top(*iter) < detail::bottom(r); });
      const auto top_line_iter = std::ranges::min_element(
        top_line_view,
        [&](const auto& r1, const auto& r2)
        {
          const auto range1 = math::value_range(detail::top(*iter), detail::bottom(r1));
          const auto range2 = math::value_range(detail::top(*iter), detail::bottom(r2));
          return decltype(range1)::size(range1) < decltype(range2)::size(range2);
        }
      );
      return top_line_iter != std::ranges::cend(top_line_view) ? *top_line_iter : *iter;
    }();
    const auto bottom_line = [&]
    {
      auto bottom_line_view =
        line_bounding_boxes |
        std::views::filter([&](const auto& r) { return r.vertical_range() > 0 && detail::bottom(*iter) > detail::top(r); });
      const auto bottom_line_iter = std::ranges::min_element(
        bottom_line_view,
        [&](const auto& r1, const auto& r2)
        {
          const auto range1 = math::value_range(detail::bottom(*iter), detail::top(r1));
          const auto range2 = math::value_range(detail::bottom(*iter), detail::top(r2));
          return decltype(range1)::size(range1) < decltype(range2)::size(range2);
        }
      );
      return bottom_line_iter != std::ranges::cend(bottom_line_view) ? *bottom_line_iter : *iter;
    }();
    const auto reduced_paragraph_bounding_box = screen_rect_type(
      {paragraph_bounding_box_iter->origin().x(), iter->origin().y()},
      paragraph_bounding_box_iter->horizontal_range(),
      iter->vertical_range()
    );
    const auto surrounding_rect = screen_rect_type::overlap(
      screen_rect_type::surrounding_rect(reduced_paragraph_bounding_box, *iter, top_line, bottom_line),
      *paragraph_bounding_box_iter
    );
    if(!surrounding_rect)
    {
      LOG_ERROR("no surrounding rectangle found for paragraph bounding box: {}", *paragraph_bounding_box_iter);
      return std::nullopt;
    }
    return recognize_bounding_box_result{*surrounding_rect, *paragraph_bounding_box_iter};
  }
  return std::nullopt;
}

///
///
auto core_bible_reference_ocr::recognize_capture_area(
  const recognize_bounding_box_result& recognized_bounding_box, std::size_t step_index
) const -> bool
{
  SCOPED_TIMER_LOG();
  static_assert(recognition_area_step_count > 0);
  if(step_index >= recognition_area_step_count)
  {
    LOG_ERROR("step index out of range: step_index={}, range=[0, {})", step_index, recognition_area_step_count);
    return false;
  }
  const auto& [initial, largest] = recognized_bounding_box;
  const auto expansion_multiplier = static_cast<double>(step_index) / static_cast<double>(recognition_area_step_count - 1);
  const auto top = static_cast<std::int32_t>(
    expansion_multiplier * std::abs(math::arithmetic::subtract(detail::top(largest), detail::top(initial)).value())
  );
  const auto bottom = static_cast<std::int32_t>(
    expansion_multiplier * std::abs(math::arithmetic::subtract(detail::bottom(initial), detail::bottom(largest)).value())
  );
  // The area is expanded only vertically with each step.
  const auto area = screen_rect_type(
    {initial.origin().x(), math::arithmetic::subtract(initial.origin().y(), bottom).value()},
    initial.horizontal_range(),
    initial.vertical_range() + bottom + top
  );
  return core_tesseract_->recognize(area);
}

///
///
auto core_bible_reference_ocr::find_main_reference_position_data(const screen_coordinates_type& relative_cursor_position) const
  -> std::optional<reference_position_data>
{
  SCOPED_TIMER_LOG();
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
  SCOPED_TIMER_LOG();
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
auto core_bible_reference_ocr::match_choices_to_bible_book(
  const std::vector<tesseract_choices>& choices_list, const std::function<bool(const tesseract_choices&)>& choices_filter
) const -> std::vector<txt::indexed_strings>
{
  auto results_unsorted = std::vector<std::pair<double, txt::indexed_strings>>{};
  std::ranges::for_each(
    bible::book_name_variants_de::name_variants_list,
    [&](const auto& element)
    {
      const auto& [_, name_variant] = element;
      const auto element_result = match_choices_to_string(choices_list, name_variant, choices_filter);
      results_unsorted.insert(results_unsorted.cend(), element_result.cbegin(), element_result.cend());
    }
  );
  std::ranges::sort(results_unsorted, [&](const auto& a, const auto& b) { return a.first > b.first; });
  auto results = std::vector<txt::indexed_strings>{};
  results.reserve(results_unsorted.size());
  std::ranges::for_each(results_unsorted, [&](auto& e) { results.emplace_back(std::move(e.second)); });
  return results;
}

///
///
auto core_bible_reference_ocr::match_choices_to_string(
  const std::vector<tesseract_choices>& choices_list,
  const std::string_view text_template,
  const std::function<bool(const tesseract_choices&)>& choices_filter
) const -> std::vector<std::pair<double, txt::indexed_strings>>
{
  auto results = std::vector<std::pair<double, txt::indexed_strings>>{};
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
        auto confidence_value = 0.0;
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
              confidence_value += choice_optional->confidence;
              local_indexed_strings.overwrite_at(choices_list_index, choice_optional->symbol);
              text_template_found = text_template_position >= text_template.size();
              return text_template_found;
            }
            return true;
          }
        );
        if(text_template_found)
        {
          results.emplace_back(confidence_value, std::move(local_indexed_strings));
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
