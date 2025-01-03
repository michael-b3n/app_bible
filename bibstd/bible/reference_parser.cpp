#include "bible/reference_parser.hpp"
#include "bible/book_name_variants_de.hpp"
#include "txt/chars.hpp"
#include "txt/find_uint.hpp"
#include "util/contains.hpp"
#include "util/string_view.hpp"

#include <cassert>
#include <ranges>

namespace bibstd::bible
{
namespace detail
{

///
///
///
auto match_passage_template_section(
  const book_id book,
  const std::span<const std::uint32_t> numbers,
  const std::string_view section,
  auto& current_level,
  std::uint32_t& current_chapter,
  const char down_transition_char) -> std::vector<reference_range>
{
  using passage_level = std::remove_reference_t<decltype(current_level)>;
  auto result = std::vector<reference_range>{};
  const auto numbers_size = numbers.size();
  if(std::format("#{}#", down_transition_char) == section && numbers_size == 2)
  {
    const auto ref = reference::create(book, numbers.at(0), numbers.at(1));
    if(ref)
    {
      result.emplace_back(reference_range(ref.value()));
    }
    current_level = passage_level::verse;
    current_chapter = numbers.at(0);
  }
  else if(std::format("#{}#-#", down_transition_char) == section && numbers_size == 3)
  {
    const auto ref1 = reference::create(book, numbers.at(0), numbers.at(1));
    const auto ref2 = reference::create(book, numbers.at(0), numbers.at(2));
    if(ref1 && ref2)
    {
      result.emplace_back(reference_range(ref1.value(), ref2.value()));
    }
    current_level = passage_level::verse;
    current_chapter = numbers.at(0);
  }
  else if(std::format("#{}#-#{}#", down_transition_char, down_transition_char) == section && numbers_size == 4)
  {
    const auto ref1 = reference::create(book, numbers.at(0), numbers.at(1));
    const auto ref2 = reference::create(book, numbers.at(2), numbers.at(3));
    if(ref1 && ref2)
    {
      result.emplace_back(reference_range(ref1.value(), ref2.value()));
    }
    current_level = passage_level::verse;
    current_chapter = numbers.at(2);
  }
  else if(std::format("#-#{}#", down_transition_char) == section && numbers_size == 3)
  {
    if(current_level == passage_level::verse)
    {
      const auto ref1 = reference::create(book, current_chapter, numbers.at(0));
      const auto ref2 = reference::create(book, numbers.at(1), numbers.at(2));
      if(ref1 && ref2)
      {
        result.emplace_back(reference_range(ref1.value(), ref2.value()));
      }
    }
    else
    {
      const auto ref1 = reference::create(book, numbers.at(0), 1u);
      const auto ref2 = reference::create(book, numbers.at(1), numbers.at(2));
      if(ref1 && ref2)
      {
        result.emplace_back(reference_range(ref1.value(), ref2.value()));
      }
      current_level = passage_level::verse;
    }
    current_chapter = numbers.at(1);
  }
  else if(std::string_view("#-#") == section && numbers_size == 2)
  {
    if(current_level == passage_level::verse)
    {
      const auto ref1 = reference::create(book, current_chapter, numbers.at(0));
      const auto ref2 = reference::create(book, current_chapter, numbers.at(1));
      if(ref1 && ref2)
      {
        result.emplace_back(reference_range(ref1.value(), ref2.value()));
      }
    }
    else
    {
      const auto ref1 = reference::create(book, numbers.at(0), 1u);
      const auto ref2 = reference::create(book, numbers.at(1), verse_count(book, numbers.at(1)).value_or(0));
      if(ref1 && ref2)
      {
        result.emplace_back(reference_range(ref1.value(), ref2.value()));
      }
      current_chapter = numbers.at(1);
    }
  }
  else if(std::string_view("#") == section && numbers_size == 1)
  {
    if(current_level == passage_level::verse)
    {
      const auto ref = reference::create(book, current_chapter, numbers.front());
      if(ref)
      {
        result.emplace_back(reference_range(ref.value()));
      }
    }
    else
    {
      const auto ref1 = reference::create(book, numbers.front(), 1u);
      const auto ref2 = reference::create(book, numbers.front(), verse_count(book, numbers.front()).value_or(0));
      if(ref1 && ref2)
      {
        result.emplace_back(reference_range(ref1.value(), ref2.value()));
      }
      current_chapter = numbers.front();
    }
  }
  return result;
}

} // namespace detail

///
///
auto reference_parser::parse(const std::string_view text) -> std::vector<reference_range>
{
  auto result = std::vector<reference_range>{};
  cache_normalized_without_whitespaces(text);
  const auto text_size = text_.size();
  for(const auto& book_id_name_variant_pair : book_name_variants_de::name_variants_list)
  {
    const auto& [book_id, name_variant] = book_id_name_variant_pair;
    auto pos = std::size_t{0};
    do
    {
      const auto text_substring = util::make_substring_view(text_, pos, text_size - pos);
      pos = text_substring.find(name_variant);
      if(pos != std::string_view::npos)
      {
        pos += name_variant.size();
        if(pos < text_size)
        {
          passage_template_.clear();
          std::vector<std::uint32_t> numbers;
          const auto passage_substring = util::make_substring_view(text_, pos, text_size - pos);

          auto passage_pos = std::size_t{0};
          constexpr auto error_chars_max = std::size_t{2};
          auto error_chars_count = std::size_t{0};
          while(passage_pos < passage_substring.size() && error_chars_count < error_chars_max)
          {
            const auto old_passage_pos = passage_pos;
            const auto number = identify_number(passage_substring, passage_pos);
            if(number)
            {
              numbers.push_back(number.value());
              passage_template_.push_back('#');
              skip_number_postfix(passage_substring, passage_pos);
            }
            const auto transition_char = identify_transition(passage_substring, passage_pos);
            if(transition_char)
            {
              passage_template_.push_back(transition_char.value());
            }

            if(old_passage_pos == passage_pos)
            {
              ++passage_pos;
              ++error_chars_count;
            }
          }
          pos += (passage_pos - error_chars_count);

          const auto found_references = match_passage_template(book_id, numbers);
          result.insert(std::cend(result), std::cbegin(found_references), std::cend(found_references));
        }
      }
    }
    while(pos != std::string_view::npos);
  }
  return result;
}

///
///
auto reference_parser::cache_normalized_without_whitespaces(const std::string_view text) -> void
{
  text_.clear();
  if(text_.capacity() < text.size())
  {
    text_.reserve(text.size());
  }
  txt::chars::for_each_char(
    text,
    [&](const auto string_view, const txt::chars::category category) -> void
    {
      switch(category)
      {
      case txt::chars::category::whitespace: break;
      case txt::chars::category::line: text_.push_back('-'); break;
      default: text_.append(string_view.data(), string_view.size()); break;
      }
    });
}

///
///
auto reference_parser::identify_number(std::string_view text, std::size_t& pos) const -> std::optional<std::uint32_t>
{
  const auto number = txt::find_uint(text.substr(pos));
  if(number)
  {
    pos += number->post_value_offset;
    return number->value;
  }
  return std::nullopt;
}

///
///
auto reference_parser::skip_number_postfix(std::string_view text, std::size_t& pos) const -> void
{
  // clang-format off
  constexpr auto ignored_sequence = std::array{
    std::string_view("ff."),
    std::string_view("f."),
    std::string_view("a"),
    std::string_view("b"),
    std::string_view("c"),
    std::string_view("d")};
  // clang-format on
  if(const auto seq = txt::chars::contains(ignored_sequence, text, pos); seq)
  {
    pos += seq->size();
  }
}

///
///
auto reference_parser::identify_transition(const std::string_view text, std::size_t& pos) const -> std::optional<char>
{
  constexpr auto transition_chars = std::array{',', ';', '.', '-', ':'};
  if(pos >= text.size())
  {
    return std::nullopt;
  }
  const auto transition_char = text.at(pos);
  if(util::contains(transition_chars, transition_char))
  {
    ++pos;
    return transition_char;
  }
  return std::nullopt;
}

///
///
auto reference_parser::match_passage_template(const book_id book, const std::vector<std::uint32_t>& numbers)
  -> std::vector<reference_range>
{
  if(!util::valid(book))
  {
    THROW_EXCEPTION(std::invalid_argument{"invalid book ID"});
  }
  auto result = std::vector<reference_range>{};
  if(numbers.empty())
  {
    result.emplace_back(reference_range(reference::create(book, 1u, 1u).value()));
  }
  else
  {
    normalize_passage_template();
    if(passage_template_normalized_.empty())
    {
      THROW_EXCEPTION(std::invalid_argument{"invalid normalized passage template"});
    }
    reference_ranges_.clear();
    for(const auto down_transition_char : down_transition_chars_)
    {
      auto current_level = passage_level::chapter;
      auto current_chapter = numbers.front();
      auto prev_char_i = std::size_t{0};
      auto prev_number_count = std::size_t{0};
      auto number_counter = std::size_t{0};
      for(const auto [index, c] : std::views::enumerate(passage_template_normalized_))
      {
        if(c == '#')
        {
          ++number_counter;
        }
        const auto is_last_index = static_cast<std::size_t>(index + 1) == passage_template_normalized_.size();
        if((c != '#' && c != '-' && c != down_transition_char) || is_last_index)
        {
          const auto section_size = index - prev_char_i + (is_last_index ? 1 : 0);
          const auto section = util::make_substring_view(passage_template_normalized_, prev_char_i, section_size);
          const auto found = detail::match_passage_template_section(
            book,
            std::span(std::next(std::cbegin(numbers), prev_number_count), number_counter - prev_number_count),
            section,
            current_level,
            current_chapter,
            down_transition_char);
          prev_char_i = 1 + index;
          prev_number_count = number_counter;

          if(found.empty())
          {
            // If parsing of a section fails, the passage parsing for the down transition char is ignored.
            reference_ranges_.erase(down_transition_char);
            break;
          }

          decltype(auto) references = reference_ranges_[down_transition_char];
          references.insert(std::cend(references), std::cbegin(found), std::cend(found));
        }
      }
    }
    auto reference_ranges_view = reference_ranges_ | std::views::filter([](const auto& p) { return !p.second.empty(); });
    reference_ranges_verse_count_.clear();
    std::ranges::for_each(
      reference_ranges_view,
      [&](const auto& pair)
      {
        const auto& [c, references] = pair;
        const auto verse_count = std::ranges::fold_left(
          references, std::uint32_t{0}, [](std::uint32_t total, const auto& ref) { return total + ref.size(); });
        reference_ranges_verse_count_.emplace_back(std::pair{c, verse_count});
      });

    const auto reference_ranges_iter =
      std::ranges::min_element(reference_ranges_verse_count_, [](const auto& a, const auto& b) { return a.second < b.second; });
    if(reference_ranges_iter != std::ranges::cend(reference_ranges_verse_count_))
    {
      result = std::move(reference_ranges_.at(reference_ranges_iter->first));
    }
  }
  return result;
}

///
///
auto reference_parser::normalize_passage_template() -> void
{
  passage_template_normalized_.clear();
  down_transition_chars_.clear();
  const auto first = std::ranges::find(passage_template_, '#');
  const auto [last, end_last] = std::ranges::find_last(passage_template_, '#');
  if(first != std::ranges::cend(passage_template_) && last != end_last)
  {
    const auto first_i = std::ranges::distance(std::ranges::cbegin(passage_template_), first);
    const auto last_i = std::ranges::distance(std::ranges::cbegin(passage_template_), last);

    constexpr auto placeholder_discrete_transition_char = '*';
    const auto trimmed_passage_template = util::make_substring_view(passage_template_, first_i, 1 + last_i - first_i);
    for(const auto [index, c] : std::views::enumerate(trimmed_passage_template))
    {
      passage_template_normalized_.push_back(c);
      if(c == '#' && index > 0)
      {
        const auto prev = trimmed_passage_template.at(index - 1);
        if(prev == '#')
        {
          passage_template_normalized_.push_back(placeholder_discrete_transition_char);
        }
      }
      else if(c != '#' && c != '-' && !util::contains(down_transition_chars_, c))
      {
        down_transition_chars_.push_back(c);
      }
    }
  }
}

} // namespace bibstd::bible
