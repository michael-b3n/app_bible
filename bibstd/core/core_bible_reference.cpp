#include "core/core_bible_reference.hpp"
#include "bible/book_name_variants_de.hpp"
#include "math/value_range.hpp"
#include "txt/chars.hpp"
#include "txt/find_uint.hpp"
#include "util/contains.hpp"
#include "util/log.hpp"
#include "util/string.hpp"
#include "util/visit_helper.hpp"

#include <algorithm>
#include <cassert>
#include <map>
#include <ranges>

namespace bibstd::core
{
namespace detail
{

///
/// Matches a generic_passage_template template section and returns the corresponding reference ranges.
///
auto match_passage_template_section(
  const bible::book_id book,
  const std::span<const std::uint32_t> numbers,
  const std::string_view section,
  auto& current_level,
  std::uint32_t& current_chapter
) -> std::vector<bible::reference_range>
{
  using passage_level = std::remove_reference_t<decltype(current_level)>;
  auto result = std::vector<bible::reference_range>{};
  const auto numbers_size = numbers.size();
  if(std::string_view("#X#-#X#") == section && numbers_size == 4)
  {
    const auto ref1 = bible::reference::create(book, numbers.at(0), numbers.at(1));
    const auto ref2 = bible::reference::create(book, numbers.at(2), numbers.at(3));
    if(ref1 && ref2)
    {
      result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
    }
    current_level = passage_level::verse;
    current_chapter = numbers.at(2);
  }
  else if(std::string_view("#X#-#") == section && numbers_size == 3)
  {
    const auto ref1 = bible::reference::create(book, numbers.at(0), numbers.at(1));
    const auto ref2 = bible::reference::create(book, numbers.at(0), numbers.at(2));
    if(ref1 && ref2)
    {
      result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
    }
    current_level = passage_level::verse;
    current_chapter = numbers.at(0);
  }
  else if(std::string_view("#-#X#") == section && numbers_size == 3)
  {
    if(current_level == passage_level::verse)
    {
      const auto ref1 = bible::reference::create(book, current_chapter, numbers.at(0));
      const auto ref2 = bible::reference::create(book, numbers.at(1), numbers.at(2));
      if(ref1 && ref2)
      {
        result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
      }
    }
    else
    {
      const auto ref1 = bible::reference::create(book, numbers.at(0), 1u);
      const auto ref2 = bible::reference::create(book, numbers.at(1), numbers.at(2));
      if(ref1 && ref2)
      {
        result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
      }
      current_level = passage_level::verse;
    }
    current_chapter = numbers.at(1);
  }
  else if(std::string_view("#X#") == section && numbers_size == 2)
  {
    const auto ref = bible::reference::create(book, numbers.at(0), numbers.at(1));
    if(ref)
    {
      result.emplace_back(bible::reference_range(ref.value()));
    }
    current_level = passage_level::verse;
    current_chapter = numbers.at(0);
  }
  else if(std::string_view("#-#") == section && numbers_size == 2)
  {
    if(current_level == passage_level::verse)
    {
      const auto ref1 = bible::reference::create(book, current_chapter, numbers.at(0));
      const auto ref2 = bible::reference::create(book, current_chapter, numbers.at(1));
      if(ref1 && ref2)
      {
        result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
      }
    }
    else
    {
      const auto ref1 = bible::reference::create(book, numbers.at(0), 1u);
      const auto ref2 = bible::reference::create(book, numbers.at(1), verse_count(book, numbers.at(1)).value_or(0));
      if(ref1 && ref2)
      {
        result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
      }
      current_chapter = numbers.at(1);
    }
  }
  else if(std::string_view("#") == section && numbers_size == 1)
  {
    if(current_level == passage_level::verse)
    {
      const auto ref = bible::reference::create(book, current_chapter, numbers.front());
      if(ref)
      {
        result.emplace_back(bible::reference_range(ref.value()));
      }
    }
    else
    {
      const auto ref1 = bible::reference::create(book, numbers.front(), 1u);
      const auto ref2 = bible::reference::create(book, numbers.front(), verse_count(book, numbers.front()).value_or(0));
      if(ref1 && ref2)
      {
        result.emplace_back(bible::reference_range(ref1.value(), ref2.value()));
      }
      current_chapter = numbers.front();
    }
  }
  return result;
}

} // namespace detail

///
///
auto core_bible_reference::parse(const std::string_view text, const std::size_t index) -> parse_result
{
  const auto book = find_book(text, index);
  if(!book)
  {
    return parse_result{};
  };
  return parse_result{
    .ranges = match_passage_template(
      book->book_id,
      create_passage_template(text.substr(book->index_range_numbers.begin, index_range_type::size(book->index_range_numbers)))
    ),
    .index_range_origin = index_range_type{book->index_range_book.begin, book->index_range_numbers.end},
  };
}

///
///
auto core_bible_reference::find_book(const std::string_view text, const std::size_t index) -> std::optional<find_book_result>
{
  auto found_book = std::optional<find_book_result>{};

  std::string letters_and_digits_only;
  letters_and_digits_only.reserve(text.size());
  std::vector<index_range_type> raw_index_ranges;
  raw_index_ranges.reserve(text.size());
  txt::chars::for_each_char(
    text,
    [&](const auto string_view, const auto pos, const txt::chars::category category) -> void
    {
      const auto letter_or_digit = category == txt::chars::category::letter || category == txt::chars::category::digit;
      const auto size = string_view.size();
      if(letter_or_digit && size > 0)
      {
        letters_and_digits_only.append(string_view.data(), size);
        const auto index_range = index_range_type{pos, pos + size};
        raw_index_ranges.insert(raw_index_ranges.end(), size, index_range);
      }
    }
  );
  assert(raw_index_ranges.size() == letters_and_digits_only.size());
  std::ranges::for_each(
    bible::book_name_variants_de::name_variants_list |
      std::views::take_while([&]([[maybe_unused]] auto&) { return !found_book.has_value(); }),
    [&](const auto& element)
    {
      auto letters_and_digits_only_view = std::string_view{letters_and_digits_only};
      const auto& [book_id, name_variant] = element;

      auto pos_rel = std::size_t{0};
      auto pos_offset = std::size_t{0};
      const auto max_iterations = letters_and_digits_only_view.size();
      std::ranges::for_each(
        std::views::iota(std::size_t{0}, max_iterations) |
          std::views::take_while([&](const auto i) { return pos_rel != std::string_view::npos && !found_book.has_value(); }),
        [&]([[maybe_unused]] const auto)
        {
          pos_rel = letters_and_digits_only_view.find(name_variant);
          if(pos_rel == std::string_view::npos)
          {
            return;
          }
          const auto pos_name_end = pos_rel + name_variant.size();
          if(txt::chars::is_char(letters_and_digits_only_view, pos_name_end, txt::chars::category::digit))
          {
            const auto text_after_pos = letters_and_digits_only_view.substr(pos_name_end);
            auto numbers_offset_opt = std::optional<std::size_t>{};
            txt::chars::for_each_char_while(
              text_after_pos,
              [&]([[maybe_unused]] auto, const auto pos, const txt::chars::category category)
              {
                if(category != txt::chars::category::digit &&
                   !util::contains(
                     number_postfix_chars, [&](const auto v) { return util::starts_with(text_after_pos.substr(pos), v); }
                   ))
                {
                  numbers_offset_opt = pos;
                }
                return !numbers_offset_opt.has_value();
              }
            );
            const auto number_offset = numbers_offset_opt.value_or(text_after_pos.size());
            const auto pos_abs = pos_offset + pos_rel;
            const auto index_begin = raw_index_ranges.at(pos_abs).begin;
            const auto index_passage_begin = raw_index_ranges.at(pos_abs + name_variant.size()).begin;
            const auto index_end = raw_index_ranges.at(pos_abs + name_variant.size() + number_offset - 1).end;
            if(math::value_range<std::size_t>::contains(index_range_type{index_begin, index_end}, index))
            {
              found_book = find_book_result{
                .book_id = book_id,
                .index_range_book = index_range_type{        index_begin, index_passage_begin},
                .index_range_numbers = index_range_type{index_passage_begin,           index_end},
                .book_name_variant = name_variant
              };
            }
          }
          pos_offset += pos_name_end;
          letters_and_digits_only_view = letters_and_digits_only_view.substr(pos_name_end);
        }
      );
    }
  );
  return found_book;
}

///
///
auto core_bible_reference::create_passage_template(const std::string_view passage_text) -> passage_template_type
{
  const auto normalized = normalize_passage_text(passage_text);
  passage_template_type passage_template;
  auto passage_substring = std::string_view{normalized};
  auto pos = std::size_t{0};

  std::optional<char> transition_char;
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, normalized.size()) |
      std::views::take_while([&]([[maybe_unused]] auto) { return pos < normalized.size(); }),
    [&]([[maybe_unused]] auto)
    {
      if(const auto number = identify_number(passage_substring, pos); number)
      {
        passage_template.push_back(number.value());
        transition_char = std::nullopt;
      }
      if(const auto transition_char = identify_transition(passage_substring, pos); transition_char)
      {
        if(!passage_template.empty() && std::holds_alternative<std::uint32_t>(passage_template.back()))
        {
          // Take first found transition chars and ignore further chars.
          passage_template.push_back(transition_char.value());
        }
      }
      else // Exit when no transition char is found.
      {
        pos = std::string_view::npos;
      }
    }
  );
  const auto is_number = [](const auto& e) { return std::holds_alternative<std::uint32_t>(e); };
  const auto first = std::ranges::find_if(passage_template, is_number);
  if(first != std::ranges::cend(passage_template))
  {
    passage_template.erase(std::ranges::cbegin(passage_template), first);
  }
  const auto [last, end_last] = std::ranges::find_last_if(passage_template, is_number);
  if(last != end_last)
  {
    passage_template.erase(std::next(last), std::ranges::cend(passage_template));
  }
  return passage_template;
}

///
///
auto core_bible_reference::normalize_passage_text(const std::string_view text) -> std::string
{
  std::string normalized_text;
  auto counter = std::size_t{0};
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, text.size()) | std::views::take_while([&](const auto i) { return counter < text.size(); }),
    [&]([[maybe_unused]] const auto)
    {
      const auto subview = text.substr(counter);
      if(const auto iter =
           std::ranges::find_if(number_postfixes, [&](const auto postfix) { return util::starts_with(subview, postfix); });
         iter != std::ranges::cend(number_postfixes))
      {
        counter += iter->size(); // Ignore possible postfixes
      }
      else if(const auto data = txt::chars::char_info(subview, 0); data)
      {
        switch(data->char_category)
        {
        case txt::chars::category::letter: break;
        case txt::chars::category::whitespace: break;
        case txt::chars::category::line: normalized_text.push_back('-'); break;
        default: normalized_text.append(subview.data(), data->char_size); break;
        }
        counter += data->char_size;
      }
      else
      {
        LOG_ERROR(log_channel, "Error getting char info: char=\'{}\'", subview.at(0));
        ++counter;
      }
    }
  );
  return normalized_text;
}

///
///
auto core_bible_reference::identify_number(std::string_view text, std::size_t& pos) const -> std::optional<std::uint32_t>
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
auto core_bible_reference::identify_transition(const std::string_view text, std::size_t& pos) const -> std::optional<char>
{
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
auto core_bible_reference::match_passage_template(const bible::book_id book, passage_template_type&& passage_template)
  -> std::vector<bible::reference_range>
{
  if(!util::valid(book))
  {
    THROW_EXCEPTION(std::invalid_argument{"invalid book ID"});
  }
  auto result = std::vector<bible::reference_range>{};
  const auto down_transition_chars = passage_template_transition_chars(passage_template);
  const auto numbers = passage_template_numbers(passage_template);
  if(passage_template.empty())
  {
    result.emplace_back(bible::reference_range(bible::reference::create(book, 1u, 1u).value()));
    return result;
  }
  else if(down_transition_chars.empty() && numbers.size() == 1)
  {
    const auto reference_begin = bible::reference::create(book, numbers.front(), 1u);
    const auto reference_end = bible::reference::create(
      book, numbers.front(), bible::verse_count(book, numbers.front()).value_or(0u /*will evaluate to std::nullopt*/)
    );
    if(reference_begin && reference_end)
    {
      result.emplace_back(bible::reference_range(*reference_begin, *reference_end));
    }
    return result;
  }

  std::map<char, std::vector<bible::reference_range>> reference_ranges;
  for(const auto down_transition_char : down_transition_chars)
  {
    const auto passage_sections = create_passage_sections(passage_template, down_transition_char);
    auto current_level = passage_level::chapter;
    auto current_chapter = numbers.front();
    std::ranges::all_of(
      passage_sections,
      [&](const auto& passage_section)
      {
        const auto found = detail::match_passage_template_section(
          book, passage_section.numbers, passage_section.generic_template, current_level, current_chapter
        );
        const auto result = !found.empty();
        if(result)
        {
          decltype(auto) ranges = reference_ranges[down_transition_char];
          ranges.insert(ranges.end(), found.cbegin(), found.cend());
        }
        else
        {
          reference_ranges.erase(down_transition_char);
        }
        return result;
      }
    );
  }

  auto reference_ranges_view = reference_ranges | std::views::filter([](const auto& p) { return !p.second.empty(); });
  std::vector<std::pair<char, std::uint32_t>> reference_ranges_verse_count;
  std::ranges::for_each(
    reference_ranges_view,
    [&](const auto& pair)
    {
      const auto& [c, references] = pair;
      const auto verse_count = std::ranges::fold_left(
        references, std::uint32_t{0}, [](std::uint32_t total, const auto& ref) { return total + ref.size(); }
      );
      reference_ranges_verse_count.emplace_back(std::pair{c, verse_count});
    }
  );
  const auto reference_ranges_iter =
    std::ranges::min_element(reference_ranges_verse_count, [](const auto& a, const auto& b) { return a.second < b.second; });
  if(reference_ranges_iter != std::ranges::cend(reference_ranges_verse_count))
  {
    result = std::move(reference_ranges.at(reference_ranges_iter->first));
  }
  return result;
}

///
///
auto core_bible_reference::passage_template_transition_chars(const passage_template_type& passage_template) -> std::vector<char>
{
  auto result = std::vector<char>{};
  std::ranges::for_each(
    passage_template | std::views::filter([](const auto e) { return std::holds_alternative<char>(e); }) |
      std::views::transform([](const auto e) { return std::get<char>(e); }) |
      std::views::filter([](const auto c) { return c != '-'; }) |
      std::views::filter([&](const auto c) { return !util::contains(result, c); }),
    [&](const auto c) { result.push_back(c); }
  );
  return result;
}

///
///
auto core_bible_reference::passage_template_numbers(const passage_template_type& passage_template) -> std::vector<std::uint32_t>
{
  auto result = std::vector<std::uint32_t>{};
  std::ranges::for_each(
    passage_template | std::views::filter([](const auto e) { return std::holds_alternative<std::uint32_t>(e); }) |
      std::views::transform([](const auto e) { return std::get<std::uint32_t>(e); }),
    [&](const auto n) { result.push_back(n); }
  );
  return result;
}

///
///
auto core_bible_reference::create_passage_sections(
  const passage_template_type& passage_template, const char down_transition_char
) -> std::vector<passage_section>
{
  const auto to_transition_char = [down_transition_char](const char c) -> std::optional<char>
  {
    if(c == '-') return '-';
    else if(c == down_transition_char) return 'X';
    else return std::nullopt;
  };

  std::vector<passage_section> result;
  passage_section current_result;
  const auto handle_uint32_t = [&](const std::uint32_t n)
  {
    current_result.numbers.push_back(n);
    current_result.generic_template.push_back('#');
  };
  const auto handle_char = [&](const char c)
  {
    if(const auto transition_char = to_transition_char(c); transition_char)
    {
      current_result.generic_template.push_back(*transition_char);
    }
    else
    {
      result.emplace_back(std::move(current_result));
      current_result = passage_section{};
    }
  };
  std::ranges::for_each(passage_template, [&](const auto e) { util::visit_lambdas(e, handle_uint32_t, handle_char); });
  result.emplace_back(std::move(current_result));
  return result;
}

} // namespace bibstd::core
