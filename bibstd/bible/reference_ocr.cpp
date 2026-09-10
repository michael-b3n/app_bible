#include "bibstd/bible/reference_ocr.hpp"
#include "bibstd/math/coordinates.hpp"
#include "bibstd/math/rect.hpp"
#include "bibstd/txt/ocr_engine.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/numeric_cast.hpp"
#include "bibstd/util/ranges.hpp"
#include "bibstd/util/screen_types.hpp"
#include "bibstd/util/timer.hpp"
#include "bibstd/util/visit_helper.hpp"

#include <algorithm>
#include <cstddef>
#include <expected>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace bibstd::bible
{
namespace
{

///
/// Concept to check if a type has a member named paragraph_data.
///
template<typename T>
concept has_paragraph_data = requires(T t) { t.paragraph_data; };

///
/// Access the name of the OCR engine.
/// \return engine name
///
[[nodiscard]] auto name(const txt::ocr_engine_uptr_variant_type& engine) -> std::string
{
  return util::visit_lambdas(
    engine,
    []([[maybe_unused]] const std::monostate&) { return std::string{"Undefined"}; },
    [](const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type& e) { return e->name(); },
    [](const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type& e) { return e->name(); }
  );
}

///
/// Consecutive words of the recognition data, given as the index range [begin, end) into it. The
/// engines report their words in reading order, so the words of one line or paragraph are consecutive.
///
struct word_range final
{
  // Variables
  std::size_t begin{0};
  std::size_t end{0};

  ///
  /// \return true if the word belongs to the range
  ///
  [[nodiscard]] auto contains(const std::size_t word_index) const -> bool { return word_index >= begin && word_index < end; }

  ///
  /// \return indices of the words in reading order
  ///
  [[nodiscard]] auto indices() const -> auto { return util::ranges::index_view_between(begin, end); }
};

///
/// This struct holds the text a reference is searched in, together
/// with the words it was built from.
///
struct position_text final
{
  ///
  /// Text one recognized line or paragraph supplied, together with the words written in it.
  ///
  struct text_run final
  {
    // Position of the run within the text.
    std::size_t begin;
    std::size_t size;
    // Words the run was written from.
    word_range words;
  };

  // Variables
  std::string text;
  std::vector<text_run> runs;
  // Words of the line that was pointed at.
  word_range focus_line_words;
};

///
/// Widen the range around the given word as long as the predicate accepts the neighbour. A word the
/// predicate rejects ends the range, so words it accepts elsewhere in the data stay out of it.
/// \return consecutive accepted words around the given one
///
auto words_around(const auto& recognition_data, const std::size_t word_index, const auto& predicate) -> word_range
{
  auto result = word_range{.begin = word_index, .end = word_index + 1};
  while(result.begin > 0 && predicate(recognition_data.at(result.begin - 1)))
  {
    --result.begin;
  }
  while(result.end < recognition_data.size() && predicate(recognition_data.at(result.end)))
  {
    ++result.end;
  }
  return result;
}

///
/// Check whether the recognized element sits on the line with the given bounding box. Every word of
/// a line carries the bounding box of its own line, so equality identifies the line.
/// \return true if the element sits on that line
///
auto on_line(const auto& element, const auto& line_bounding_box) -> bool
{
  return element.line_data && element.line_data->bounding_box == line_bounding_box;
}

///
/// Take the text of one recognized element as a whole, written by the given words.
/// \return text with a single run over all of it
///
auto whole_of(const auto& element_text, const word_range words) -> position_text
{
  return position_text{
    .text = element_text,
    .runs = {position_text::text_run{.begin = 0, .size = element_text.size(), .words = words}},
    .focus_line_words = {}
  };
}

///
/// Take the text of the paragraph the cursor word sits in. Only an engine with layout analysis
/// reports paragraphs at all.
/// \return text of the paragraph and the words written in it, nothing without a paragraph
///
auto paragraph_text(const auto& recognition_data, const std::size_t cursor_word_index) -> std::optional<position_text>
{
  const auto& cursor_element = recognition_data.at(cursor_word_index);
  if constexpr(has_paragraph_data<decltype(cursor_element)>)
  {
    if(cursor_element.paragraph_data)
    {
      const auto& paragraph_bounding_box = cursor_element.paragraph_data->bounding_box;
      const auto in_paragraph = [&](const auto& e)
      { return e.paragraph_data && e.paragraph_data->bounding_box == paragraph_bounding_box; };
      return whole_of(cursor_element.paragraph_data->text, words_around(recognition_data, cursor_word_index, in_paragraph));
    }
  }
  return std::nullopt;
}

///
/// One recognized line together with the words written on it.
///
template<typename LineType>
struct line_words final
{
  std::reference_wrapper<const LineType> line;
  word_range words;
};

///
/// Group the recognized words into the lines they sit on. Every word carries the line it sits on and
/// the words are reported in reading order, so a change of the line ends the current group.
/// \return lines in reading order, each with the words written on it
///
auto collect_lines(const auto& recognition_data) -> auto
{
  using element_type = std::ranges::range_value_t<std::remove_cvref_t<decltype(recognition_data)>>;
  using line_type = typename decltype(element_type::line_data)::value_type;

  auto result = std::vector<line_words<line_type>>{};
  std::ranges::for_each(
    recognition_data | std::views::enumerate,
    [&](const auto& p)
    {
      const auto& [index, element] = p;
      if(!element.line_data)
      {
        return;
      }
      const auto word_index = static_cast<std::size_t>(index);
      const auto words = word_range{.begin = word_index, .end = word_index + 1};
      if(result.empty() || result.back().line.get().bounding_box != element.line_data->bounding_box)
      {
        result.emplace_back(*element.line_data, words);
      }
      else
      {
        result.back().words.end = words.end;
      }
    }
  );
  return result;
}

///
/// Join the text of the given lines, each of them terminated by exactly one '\n'.
/// \return text of the lines with one run per line
///
auto text_of_lines(const auto& lines) -> position_text
{
  auto result = position_text{};
  std::ranges::for_each(
    lines,
    [&](const auto& l)
    {
      // Terminate every line with exactly one '\n'. The engines differ in whether they terminate their
      // line text at all and in the break characters they use, and without a break the last word of a
      // line and the first word of the next one would be read as a single word.
      auto line_text = std::string_view{l.line.get().text};
      while(line_text.ends_with('\n') || line_text.ends_with('\r'))
      {
        line_text.remove_suffix(1);
      }
      const auto begin = result.text.size();
      result.text.append(line_text);
      result.text.push_back('\n');
      result.runs.emplace_back(position_text::text_run{.begin = begin, .size = result.text.size() - begin, .words = l.words});
    }
  );
  return result;
}

///
/// Collect the text belonging to the recognized element the position falls into. An engine with layout
/// analysis groups the lines into paragraphs itself, an engine without reports lines only. There the line
/// above and below are taken as well, otherwise a reference broken over a line break is cut in half.
/// \return text around the position, the words it was taken from and the words of the pointed at line
///
auto text_around(const auto& recognition_data, const std::size_t cursor_word_index) -> position_text
{
  const auto& cursor_element = recognition_data.at(cursor_word_index);

  // Words of the line the cursor sits on. Without line data only the word pointed at is left.
  const auto focus_line_words =
    cursor_element.line_data
      ? words_around(
          recognition_data, cursor_word_index, [&](const auto& e) { return on_line(e, cursor_element.line_data->bounding_box); }
        )
      : word_range{.begin = cursor_word_index, .end = cursor_word_index + 1};
  const auto with_focus_line = [&](position_text text)
  {
    text.focus_line_words = focus_line_words;
    return text;
  };

  if(auto paragraph = paragraph_text(recognition_data, cursor_word_index))
  {
    return with_focus_line(std::move(*paragraph));
  }
  if(!cursor_element.line_data)
  {
    return with_focus_line(whole_of(cursor_element.word_data.text, focus_line_words));
  }

  const auto lines = collect_lines(recognition_data);
  const auto cursor_line = std::ranges::find_if(lines, [&](const auto& l) { return l.words.contains(cursor_word_index); });
  if(cursor_line == std::ranges::cend(lines))
  {
    LOG_WARN("cursor line not found in lines: cursor line bounding box: {}", cursor_element.line_data->bounding_box);
    return with_focus_line(whole_of(cursor_element.line_data->text, focus_line_words));
  }

  // clang-format off
  const auto first = cursor_line == std::ranges::cbegin(lines) ? cursor_line : std::ranges::prev(cursor_line);
  const auto end = std::ranges::next(cursor_line) == std::ranges::cend(lines) ? std::ranges::cend(lines) : std::ranges::next(cursor_line, 2);
  // clang-format on
  return with_focus_line(text_of_lines(std::ranges::subrange{first, end}));
}

///
/// Position of one character: the word that supplied it and its bounding box.
///
using character_position_type = std::optional<std::pair<std::size_t, typename txt::ocr_engine<>::bounding_box_type>>;

///
/// Approximate the bounding box of every character of the word by dividing the bounding box of the
/// word into equally wide parts.
/// \return one bounding box per character of the word in reading order
///
auto character_boxes_of(const auto& word) -> std::vector<std::remove_cvref_t<decltype(word.bounding_box)>>
{
  const auto word_h_range = math::size(word.bounding_box.horizontal_range());
  const auto char_width_approx = word_h_range / numeric_cast<decltype(word_h_range)>(word.text.size());
  const auto char_width_approx_signed = numeric_cast<std::make_signed_t<decltype(char_width_approx)>>(char_width_approx);
  const auto word_origin = word.bounding_box.origin();

  return util::ranges::index_view_to(word.text.size()) |
         std::views::transform(
           [&](const auto i)
           {
             const auto shifted_x = word_origin.x() + (i * char_width_approx_signed);
             return std::remove_cvref_t<decltype(word.bounding_box)>{
               decltype(word_origin){shifted_x, word_origin.y()},
               char_width_approx,
               math::size(word.bounding_box.vertical_range())
             };
           }
         ) |
         std::ranges::to<std::vector>();
}

///
/// Generate character positions by matching every word of a text run to the text the run holds. A word
/// is only looked for in the run it supplied its text to. A word of a neighboring line would otherwise
/// match a word of the same spelling anywhere in the text and drag the search past everything that
/// follows it.
/// \return list of character positions, one for each char of the text
///
auto generate_character_positions(const auto& recognition_data, const position_text& position)
  -> std::vector<character_position_type>
{
  auto result = std::vector<character_position_type>(position.text.size());

  std::ranges::for_each(
    position.runs,
    [&](const auto& run)
    {
      const auto run_text = std::string_view{position.text}.substr(run.begin, run.size);

      // The words are searched in reading order and each one continues behind its predecessor, so a
      // word repeated within the run takes the occurrence belonging to it.
      auto search_begin = std::size_t{0};
      std::ranges::for_each(
        run.words.indices(),
        [&](const auto word_index)
        {
          const auto& word = recognition_data.at(word_index).word_data;
          // A word without text supplies no character and cannot be located.
          if(word.text.empty())
          {
            return;
          }
          const auto pos = run_text.find(std::string_view{word.text}, search_begin);
          if(pos == std::string_view::npos)
          {
            LOG_WARN("expected word not found: \"{}\"", word.text);
            return;
          }
          search_begin = pos + word.text.size();

          const auto boxes = character_boxes_of(word);
          const auto word_begin = run.begin + pos;
          std::ranges::for_each(
            util::ranges::index_view(boxes), [&](const auto i) { result.at(word_begin + i) = {word_index, boxes.at(i)}; }
          );
        }
      );
    }
  );
  return result;
}

///
/// Find the word the position points at. A position between two words falls into no word, there the
/// line it points at is taken.
/// \return word pointed at as index into the recognition data, nothing if the position points nowhere
///
auto find_cursor_word(const auto& recognition_data, const reference_ocr::position_type position) -> std::optional<std::size_t>
{
  const auto index_of = [&](const auto& it)
  { return static_cast<std::size_t>(std::ranges::distance(std::ranges::cbegin(recognition_data), it)); };
  const auto in_word = [&](const auto& d) { return math::contains(d.word_data.bounding_box, position); };
  const auto in_line = [&](const auto& d) { return d.line_data && math::contains(d.line_data->bounding_box, position); };

  if(const auto it = std::ranges::find_if(recognition_data, in_word); it != std::ranges::cend(recognition_data))
  {
    return index_of(it);
  }
  if(const auto it = std::ranges::find_if(recognition_data, in_line); it != std::ranges::cend(recognition_data))
  {
    return index_of(it);
  }
  return std::nullopt;
}

///
/// Search the character closest to the position among those the words of the focus line supplied.
/// Only a character of the line that was pointed at can be the one under the cursor.
/// \return index of the character within the text, nothing if no such character was located
///
auto find_closest_character(
  const std::vector<character_position_type>& character_positions,
  const word_range focus_line_words,
  const reference_ocr::position_type position
) -> std::optional<std::size_t>
{
  const auto to_distance = [&](const auto& p)
  {
    return p && focus_line_words.contains(p->first) ? reference_ocr::position_type::distance(p->second.center(), position)
                                                    : std::numeric_limits<double>::max();
  };
  const auto distance_view = character_positions | std::views::transform(to_distance);
  const auto it = std::ranges::min_element(distance_view, std::less{});
  // Without a single located character on that line there is no cursor index. Taking the closest
  // one anyway lands on the first character of the text, a reference that is never pointed at.
  if(it == std::ranges::cend(distance_view) || *it == std::numeric_limits<double>::max())
  {
    return std::nullopt;
  }
  return static_cast<std::size_t>(std::ranges::distance(std::ranges::cbegin(distance_view), it));
}

///
/// Find index corresponding to char within recognized in text using position and bounding box data.
/// \return text and index as expected result and an error code as and unexpected result.
///
auto find_index(const auto& recognition_data, const reference_ocr::position_type position)
  -> std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>
{
  if(recognition_data.empty())
  {
    LOG_DEBUG("returns empty: recognition_data is empty");
    return reference_ocr::reference_position_data{};
  }
  const auto cursor_word_index = find_cursor_word(recognition_data, position);
  if(!cursor_word_index)
  {
    LOG_DEBUG("returns empty: position is not contained in any word bounding box");
    return reference_ocr::reference_position_data{};
  }

  const auto position_data = text_around(recognition_data, *cursor_word_index);
  const auto& text = position_data.text;
  const auto character_positions = generate_character_positions(recognition_data, position_data);
  assert(character_positions.size() == text.size());

  const auto index = find_closest_character(character_positions, position_data.focus_line_words, position);
  if(!index)
  {
    LOG_DEBUG("returns empty: no character of the pointed at line was located in the recognized text");
    return reference_ocr::reference_position_data{};
  }
  LOG_DEBUG(
    "returns reference position data: text=\"{}[{}]{}\"", text.subview(0, *index), text.at(*index), text.subview(*index + 1)
  );
  auto boxes = character_positions |
               std::views::transform([](const auto& p) { return p ? std::make_optional(p->second) : std::nullopt; }) |
               std::ranges::to<std::vector>();
  return reference_ocr::reference_position_data{text, *index, std::move(boxes)};
}

///
/// Shift all character bounding boxes of the position data by the specified offset.
/// This is needed to convert bounding boxes that are relative to a recognition
/// subarea into the coordinate system of the whole image.
/// \return position data with shifted character bounding boxes
///
auto shift_character_bounding_boxes(
  std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result> position_data,
  const reference_ocr::position_type offset
) -> std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>
{
  if(position_data)
  {
    std::ranges::for_each(
      position_data->character_bounding_boxes | std::views::filter([](const auto& box) { return box.has_value(); }),
      [&](auto& box)
      {
        using box_type = typename std::remove_reference_t<decltype(box)>::value_type;
        box = box_type{box->origin() + offset, math::size(box->horizontal_range()), math::size(box->vertical_range())};
      }
    );
  }
  return position_data;
}

///
/// Get character recognition engine from engine list using the specified name.
/// \return a reference wrapper of the engine as expected result or an error code as unexpected result.
///
auto get_character_recognition_engine(
  const reference_ocr::ocr_engine_list_type& engines, const reference_ocr::algorithm_data& ad
) -> std::expected<std::reference_wrapper<const txt::ocr_engine_uptr_variant_type>, reference_ocr::unexpected_ocr_result>
{
  const auto character_recognition_engine_it =
    std::ranges::find_if(engines, [&](const auto& e) { return name(e) == ad.engine_name_character_recognition; });
  if(character_recognition_engine_it == std::ranges::cend(engines))
  {
    LOG_ERROR("ocr engine for character recognition not found: required=\"{}\"", ad.engine_name_character_recognition);
    return std::unexpected{reference_ocr::unexpected_ocr_result::error};
  }
  return std::ref(*character_recognition_engine_it);
}

///
/// Take the area of the given line together with the line above and below it, as far as they belong
/// to the same paragraph. A reference broken over a line break is only found completely if the line
/// it continues on is recognized as well.
/// \return area of the relevant lines with a bit of padding around them
///
auto relevant_lines_area(const auto& layouts, const auto& relevant_line_it) -> util::screen_rect_type
{
  const auto& relevant_line = *relevant_line_it;
  const auto same_paragraph_line = [&](const auto& it)
  {
    return it->paragraph_bounding_box == relevant_line.paragraph_bounding_box ? it->line_bounding_box
                                                                              : relevant_line.line_bounding_box;
  };
  const auto prev = relevant_line_it == std::ranges::cbegin(layouts) ? relevant_line.line_bounding_box
                                                                     : same_paragraph_line(std::ranges::prev(relevant_line_it));
  const auto next = std::ranges::next(relevant_line_it) == std::ranges::cend(layouts)
                      ? relevant_line.line_bounding_box
                      : same_paragraph_line(std::ranges::next(relevant_line_it));
  const auto surrounding_rect = math::surrounding_rect(prev, relevant_line.line_bounding_box, next);

  // Add padding to make the recognition area a bit larger. This
  // helps OCR engines to recognize character positions better.
  const auto padding_size = math::size(relevant_line.line_bounding_box.vertical_range()) / 2;
  return util::screen_rect_type{
    math::coordinates(
      surrounding_rect.origin().x() - numeric_cast<util::screen_rect_type::value_type>(padding_size),
      surrounding_rect.origin().y() - numeric_cast<util::screen_rect_type::value_type>(padding_size)
    ),
    math::size(surrounding_rect.horizontal_range()) + (2 * padding_size),
    math::size(surrounding_rect.vertical_range()) + (2 * padding_size)
  };
}

///
/// Run paragraph recognition ony image. This requires an engine that supports layout analysis.
/// If a paragraph is found the line corresponding to the position is taken and the area of this
/// line, the previous and the next line is returned to reduce the relevant area even more.
/// \return bounding box with the relevant lines or an error code if something unexpected happens
///
auto run_paragraph_recognition(
  const reference_ocr::ocr_engine_list_type& engines,
  const reference_ocr::pixel_plane_view_type& image,
  const reference_ocr::position_type position,
  const reference_ocr::algorithm_data& ad
) -> std::expected<util::screen_rect_type, reference_ocr::unexpected_ocr_result>
{
  SCOPED_TIMER_LOG();
  using return_type = std::expected<util::screen_rect_type, reference_ocr::unexpected_ocr_result>;

  if(!ad.engine_name_layout_recognition)
  {
    LOG_ERROR("ocr engine for paragraph recognition not specified");
    return std::unexpected{reference_ocr::unexpected_ocr_result::error};
  }
  const auto engine_it =
    std::ranges::find_if(engines, [&](const auto& e) { return name(e) == ad.engine_name_layout_recognition; });
  if(engine_it == std::ranges::cend(engines))
  {
    LOG_ERROR("ocr engine for paragraph recognition not found: required=\"{}\"", *ad.engine_name_layout_recognition);
    return std::unexpected{reference_ocr::unexpected_ocr_result::error};
  }
  const auto& engine_variant = *engine_it;
  return util::visit_lambdas(
    engine_variant,
    [&]([[maybe_unused]] const std::monostate&) -> return_type
    {
      LOG_ERROR("undefined ocr engine does not support paragraph recognition");
      return std::unexpected{reference_ocr::unexpected_ocr_result::error};
    },
    [&](const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type& e) -> return_type
    {
      LOG_ERROR("ocr engine \"{}\" does not support paragraph recognition", e->name());
      return std::unexpected{reference_ocr::unexpected_ocr_result::error};
    },
    [&](const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type& e) -> return_type
    {
      auto& engine = *e;
      engine.initialize(image, std::nullopt);
      const auto layouts = engine.layout_analysis();
      const auto relevant_line_it =
        std::ranges::find_if(layouts, [&](const auto& line) { return math::contains(line.line_bounding_box, position); });
      if(relevant_line_it == std::ranges::cend(layouts))
      {
        LOG_DEBUG("paragraph recognition returns with empty rect");
        return util::screen_rect_type{math::coordinates(0, 0), 0u, 0u};
      }
      if(!relevant_line_it->paragraph_bounding_box)
      {
        return relevant_line_it->line_bounding_box;
      }
      return relevant_lines_area(layouts, relevant_line_it);
    }
  );
}

///
/// Recognize text in image using layout analysis and recognition on the reduced area
/// \see run_paragraph_recognition.
/// \return text with character index corresponding to the specified position
///
auto recognize_with_paragraph_recognition(
  const reference_ocr::ocr_engine_list_type& engines,
  const reference_ocr::pixel_plane_view_type& image,
  const reference_ocr::position_type& position,
  const reference_ocr::algorithm_data& ad
) -> std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>
{
  using return_type = std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>;
  const auto engine_ref = get_character_recognition_engine(engines, ad);
  if(!engine_ref)
  {
    return std::unexpected{engine_ref.error()};
  }
  const auto area = run_paragraph_recognition(engines, image, position, ad);
  if(!area)
  {
    return std::unexpected{area.error()};
  }

  // The padding added around the lines can reach outside the image. An engine recognizes the
  // area clipped to the image, so the character boxes it reports are relative to the clipped
  // area and it is that origin the boxes have to be shifted back by.
  const auto clipped = math::overlap(*area, util::screen_rect_type{math::coordinates(0, 0), image.width(), image.height()});
  if(!clipped || math::empty(*clipped))
  {
    // empty position data
    return reference_ocr::reference_position_data{};
  }

  const auto origin = clipped->origin();
  const auto relative_position = position - origin;
  return util::visit_lambdas(
    engine_ref->get(),
    []([[maybe_unused]] const std::monostate&) -> return_type
    { return std::unexpected{reference_ocr::unexpected_ocr_result::error}; },
    [&](const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type& e) -> return_type
    {
      SCOPED_TIMER_LOG();
      e->initialize(image, *clipped);
      return shift_character_bounding_boxes(find_index(e->recognize(), relative_position), origin);
    },
    [&](const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type& e) -> return_type
    {
      SCOPED_TIMER_LOG();
      e->initialize(image, *clipped);
      return shift_character_bounding_boxes(find_index(e->recognize(), relative_position), origin);
    }
  );
}

///
/// Recognize text in image using no analysis analysis and directly the character recognition OCR engine.
/// This method yields imprecise results but might detect more hidden text fields.
/// \return text with character index corresponding to the specified position
///
///
auto recognize_just_with_line_recognition(
  const reference_ocr::ocr_engine_list_type& engines,
  const reference_ocr::pixel_plane_view_type& image,
  const reference_ocr::position_type position,
  const reference_ocr::algorithm_data& ad
) -> std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>
{
  using return_type = std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>;
  const auto engine_ref = get_character_recognition_engine(engines, ad);
  if(!engine_ref)
  {
    return std::unexpected{engine_ref.error()};
  }
  return util::visit_lambdas(
    engine_ref->get(),
    []([[maybe_unused]] const std::monostate&) -> return_type
    { return std::unexpected{reference_ocr::unexpected_ocr_result::error}; },
    [&](const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type& e) -> return_type
    {
      e->initialize(image, std::nullopt);
      return find_index(e->recognize(), position);
    },
    [&](const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type& e) -> return_type
    {
      e->initialize(image, std::nullopt);
      return find_index(e->recognize(), position);
    }
  );
}

} // namespace

///
///
auto reference_ocr::run(
  const ocr_engine_list_type& engines,
  const pixel_plane_view_type& image,
  const position_type position,
  const algorithm_data& ad
) -> std::expected<reference_position_data, unexpected_ocr_result>
{
  switch(ad.algorithm)
  {
  case algorithm_type::recognize_with_paragraph_recognition:
    return recognize_with_paragraph_recognition(engines, image, position, ad);
  case algorithm_type::recognize_just_with_line_recognition:
    return recognize_just_with_line_recognition(engines, image, position, ad);
  default: return std::unexpected{unexpected_ocr_result::unsupported};
  }
}

} // namespace bibstd::bible
