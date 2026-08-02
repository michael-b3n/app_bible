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
#include <string_view>
#include <type_traits>
#include <variant>

namespace bibstd::bible
{
namespace detail
{

///
/// Concept to check if a type has a member named paragraph_data.
///
template<typename T>
concept has_paragraph_data = requires(T t) { t.paragraph_data; };

///
/// Engine checker to check if engine supports layout analysis.
/// \return true if layout analysis is supported and false otherwise
///
[[nodiscard]] constexpr auto supports_layout_analysis(const txt::ocr_engine_uptr_variant_type& engine) -> bool
{
  return util::visit_lambdas(
    engine,
    []([[maybe_unused]] const std::monostate&) { return false; },
    []([[maybe_unused]] const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type&) { return false; },
    []([[maybe_unused]] const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type&) { return true; }
  );
}

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
/// Checks if algorithms data is a valid struct.
/// \return true if valid, false otherwise
///
auto is_valid(const reference_ocr::algorithm_data& data) -> bool
{
  auto result = true;
  if(data.algorithm == decltype(data.algorithm)::recognize_with_paragraph_recognition)
  {
    result = result && data.engine_name_layout_recognition.has_value();
  }
  return result;
}

///
/// Generate character positions by matching the word bounding boxes to the text bounding box.
/// \return list of bounding boxes for each char
///
auto generate_character_positions(const auto& recognition_data, std::string_view text, const auto& bounding_box)
  -> std::vector<std::optional<std::pair<std::size_t, typename txt::ocr_engine<>::bounding_box_type>>>
{
  auto result = std::vector<std::optional<std::pair<std::size_t, typename txt::ocr_engine<>::bounding_box_type>>>(text.size());
  auto current_text = text;
  auto current_text_offset = std::size_t{0};

  const auto overlapping_word_data = [&](const auto& p)
  {
    const auto [data_index, data] = p;
    return math::overlap(bounding_box, data.word_data.bounding_box).has_value() && !data.word_data.text.empty();
  };

  std::ranges::for_each(
    recognition_data | std::views::enumerate | std::views::filter(overlapping_word_data),
    [&](const auto& p)
    {
      const auto [data_index, data] = p;
      const auto pos = current_text.find(std::string_view{data.word_data.text});
      if(pos != std::string_view::npos)
      {
        current_text = current_text.substr(pos);
        current_text_offset += pos;

        const auto word_size = data.word_data.text.size();
        assert(!data.word_data.text.empty());
        const auto word_h_range = math::size(data.word_data.bounding_box.horizontal_range());

        // Approximate the char width for each word as if they were all equal.
        const auto char_width_approx = word_h_range / numeric_cast<decltype(word_h_range)>(word_size);
        const auto char_width_approx_signed = numeric_cast<std::make_signed_t<decltype(char_width_approx)>>(char_width_approx);

        const auto word_origin = data.word_data.bounding_box.origin();
        std::ranges::for_each(
          util::ranges::index_view_between(current_text_offset, std::min(current_text_offset + word_size, result.size())),
          [&](const auto i)
          {
            const auto shifted_x = word_origin.x() + ((i - current_text_offset) * char_width_approx_signed);
            auto box = decltype(data.word_data.bounding_box){
              decltype(word_origin){shifted_x, word_origin.y()},
              char_width_approx,
              math::size(data.word_data.bounding_box.vertical_range())
            };
            result.at(i) = {data_index, std::move(box)};
          }
        );
      }
      else
      {
        LOG_WARN("expected word not found: \"{}\"", data.word_data.text);
      }
    }
  );
  return result;
}

///
/// Find index corresponding to char within recognized in text using position and bounding box data.
/// \return text and index as expected result and an error code as and unexpected result.
///
auto find_index(const auto& recognition_data, const reference_ocr::position_type position)
  -> std::expected<reference_ocr::reference_position_data, reference_ocr::unexpected_ocr_result>
{
  using recognition_data_value_type = typename std::remove_cvref_t<decltype(recognition_data)>::value_type;
  if(recognition_data.empty())
  {
    LOG_DEBUG("returns empty: recognition_data is empty");
    return reference_ocr::reference_position_data{};
  }

  const auto data_ref = [&]() -> std::optional<std::reference_wrapper<const recognition_data_value_type>>
  {
    const auto find_word = [&](const auto& d) { return math::contains(d.word_data.bounding_box, position); };
    const auto find_line = [&](const auto& d) { return d.line_data && math::contains(d.line_data->bounding_box, position); };
    if(const auto it_w = std::ranges::find_if(recognition_data, find_word); it_w != std::ranges::cend(recognition_data))
    {
      return std::ref(*it_w);
    }
    else if(const auto it_l = std::ranges::find_if(recognition_data, find_line); it_l != std::ranges::cend(recognition_data))
    {
      return std::ref(*it_l);
    }
    else
    {
      return std::nullopt;
    }
  }();

  if(data_ref)
  {
    const auto& data = data_ref->get();
    const auto element = [&]()
    {
      if constexpr(has_paragraph_data<decltype(data)>)
      {
        if(data.paragraph_data)
        {
          return std::pair{std::ref(data.paragraph_data->text), std::ref(data.paragraph_data->bounding_box)};
        }
      }
      if(data.line_data)
      {
        return std::pair{std::ref(data.line_data->text), std::ref(data.line_data->bounding_box)};
      }
      else
      {
        return std::pair{std::ref(data.word_data.text), std::ref(data.word_data.bounding_box)};
      }
    }();
    decltype(auto) text = element.first.get();
    static_assert(std::is_same_v<decltype(text), const std::string&>);
    decltype(auto) bounding_box = element.second.get();
    const auto character_bounding_boxes = generate_character_positions(recognition_data, text, bounding_box);
    assert(character_bounding_boxes.size() == text.size());
    const auto to_distance = [&](const auto& p)
    {
      if(p)
      {
        const auto& [i, bounding_box] = *p;
        if(recognition_data.at(i).line_data && math::contains(recognition_data.at(i).line_data->bounding_box, position))
        {
          return decltype(position)::distance(bounding_box.center(), position);
        }
      }
      return std::numeric_limits<double>::max();
    };
    const auto recognition_data_view = character_bounding_boxes | std::views::transform(to_distance);
    const auto it = std::ranges::min_element(recognition_data_view, std::less{});
    const auto distance = std::ranges::distance(std::ranges::cbegin(recognition_data_view), it);
    if(!text.empty())
    {
      LOG_DEBUG(
        "returns reference position data: text=\"{}[{}]{}\"",
        text.subview(0, distance),
        text.at(distance),
        text.subview(std::min(static_cast<std::size_t>(distance + 1), text.size() - 1), distance)
      );
    }
    return reference_ocr::reference_position_data{text, static_cast<std::size_t>(distance)};
  }
  else
  {
    LOG_DEBUG("returns empty: position is not contained in any word bounding box");
    return reference_ocr::reference_position_data{};
  }
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
    std::ranges::find_if(engines, [&](const auto& e) { return detail::name(e) == ad.engine_name_character_recognition; });
  if(character_recognition_engine_it == std::ranges::cend(engines))
  {
    LOG_ERROR("ocr engine for character recognition not found: required=\"{}\"", ad.engine_name_character_recognition);
    return std::unexpected{reference_ocr::unexpected_ocr_result::error};
  }
  return std::ref(*character_recognition_engine_it);
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
    std::ranges::find_if(engines, [&](const auto& e) { return detail::name(e) == ad.engine_name_layout_recognition; });
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
      if(relevant_line_it != std::ranges::cend(layouts))
      {
        const auto& relevant_line = *relevant_line_it;

        auto prev = relevant_line.line_bounding_box;
        auto main = relevant_line.line_bounding_box;
        auto next = relevant_line.line_bounding_box;

        if(relevant_line.paragraph_bounding_box)
        {
          if(relevant_line_it != std::ranges::cbegin(layouts))
          {
            const auto& prev_line = *std::ranges::prev(relevant_line_it);
            if(prev_line.paragraph_bounding_box == relevant_line.paragraph_bounding_box)
            {
              prev = prev_line.line_bounding_box;
            }
          }
          if(std::ranges::next(relevant_line_it) != std::ranges::cend(layouts))
          {
            const auto& next_line = *std::ranges::next(relevant_line_it);
            if(next_line.paragraph_bounding_box == relevant_line.paragraph_bounding_box)
            {
              next = next_line.line_bounding_box;
            }
          }

          const auto surrounding_rect = math::surrounding_rect(prev, main, next);

          // Add padding to make the recognition area a bit larger. This
          // helps OCR engines to recognize character positions better.
          const auto padding_size = math::size(relevant_line.line_bounding_box.vertical_range()) / 2;
          return decltype(surrounding_rect){
            math::coordinates(
              surrounding_rect.origin().x() - numeric_cast<decltype(surrounding_rect)::value_type>(padding_size),
              surrounding_rect.origin().y() - numeric_cast<decltype(surrounding_rect)::value_type>(padding_size)
            ),
            math::size(surrounding_rect.horizontal_range()) + 2 * padding_size,
            math::size(surrounding_rect.vertical_range()) + 2 * padding_size
          };
        }
        else
        {
          return relevant_line.line_bounding_box;
        }
      }
      else
      {
        LOG_DEBUG("paragraph recognition returns with empty rect");
        return util::screen_rect_type{math::coordinates(0, 0), 0u, 0u};
      }
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
  if(const auto& engine_ref = get_character_recognition_engine(engines, ad))
  {
    const auto& engine = engine_ref->get();
    if(const auto area = run_paragraph_recognition(engines, image, position, ad))
    {
      if(!math::empty(*area))
      {
        const auto relative_position = position - area->origin();
        return util::visit_lambdas(
          engine,
          []([[maybe_unused]] const std::monostate&) -> return_type
          { return std::unexpected{reference_ocr::unexpected_ocr_result::error}; },
          [&](const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type& e) -> return_type
          {
            SCOPED_TIMER_LOG();
            e->initialize(image, *area);
            return find_index(e->recognize(), relative_position);
          },
          [&](const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type& e) -> return_type
          {
            SCOPED_TIMER_LOG();
            e->initialize(image, *area);
            return find_index(e->recognize(), relative_position);
          }
        );
      }
      else
      {
        // empty position data
        return reference_ocr::reference_position_data{};
      }
    }
    else
    {
      return std::unexpected{area.error()};
    }
  }
  else
  {
    return std::unexpected{engine_ref.error()};
  }
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
  if(const auto& engine_ref = get_character_recognition_engine(engines, ad))
  {
    const auto& engine = engine_ref->get();
    return util::visit_lambdas(
      engine,
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
  else
  {
    return std::unexpected{engine_ref.error()};
  }
}

} // namespace detail

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
    return detail::recognize_with_paragraph_recognition(engines, image, position, ad);
  case algorithm_type::recognize_just_with_line_recognition:
    return detail::recognize_just_with_line_recognition(engines, image, position, ad);
  default: return std::unexpected{unexpected_ocr_result::unsupported};
  }
}

} // namespace bibstd::bible
