#include "bible/reference_parser.hpp"
#include "bible/book_name_variants_de.hpp"
#include "txt/chars.hpp"
#include "txt/find_uint.hpp"
#include "util/contains.hpp"
#include "util/string_view.hpp"

#include <ranges>

namespace bibstd::bible
{

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
          constexpr auto error_chars_max = std::size_t{4};
          auto error_chars_count = std::size_t{0};
          while(passage_pos < passage_substring.size() && error_chars_count < error_chars_max)
          {
            const auto old_passage_pos = passage_pos;
            const auto number = identify_number(passage_substring, passage_pos);
            if(number)
            {
              numbers.push_back(number.value());
              passage_template_.append("{}");
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
      default: text_.append(string_view.data()); break;
      }
    });
}

///
///
auto reference_parser::identify_number(std::string_view text, std::size_t& pos) const -> std::optional<std::uint32_t>
{
  const auto number = txt::find_uint(text);
  if(number)
  {
    pos += number->post_value_offset;
    return number->value;
  }
  return std::nullopt;
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
  auto result = std::vector<reference_range>{};
  const auto first_cb = std::ranges::find(passage_template_, '{');
  const auto [last_cb, end_last_cb] = std::ranges::find_last(passage_template_, '}');
  if(first_cb != std::ranges::cend(passage_template_) && last_cb != end_last_cb)
  {
    const auto first_cb_index = std::ranges::distance(std::ranges::cbegin(passage_template_), first_cb);
    const auto last_cb_index = std::ranges::distance(std::ranges::cbegin(passage_template_), last_cb);
    const auto passage_template =
      util::make_substring_view(passage_template_, first_cb_index, 1 + last_cb_index - first_cb_index);

    if(passage_template == std::string_view("{}"))
    {
      const auto ref = reference::create(book, numbers.at(0), static_cast<std::uint32_t>(1));
      if(ref)
      {
        result.push_back(reference_range(ref.value()));
      }
      else
      {
        LOG_WARN(log_channel, "Invalid reference: book=[{}], chapter=[{}]", book, numbers.at(0));
      }
    }
    else if(
      passage_template == std::string_view("{},{}") || passage_template == std::string_view("{}.{}") ||
      passage_template == std::string_view("{};{}") || passage_template == std::string_view("{}:{}"))
    {
      const auto ref = reference::create(book, numbers.at(0), numbers.at(1));
      if(ref)
      {
        result.push_back(reference_range(ref.value()));
      }
      else
      {
        LOG_WARN(log_channel, "Invalid reference: book=[{}], chapter=[{}], verse=[{}]", book, numbers.at(0), numbers.at(1));
      }
    }
    else if(
      passage_template == std::string_view("{},{}-{}") || passage_template == std::string_view("{}.{}-{}") ||
      passage_template == std::string_view("{};{}-{}") || passage_template == std::string_view("{}:{}-{}"))
    {
      const auto ref1 = reference::create(book, numbers.at(0), numbers.at(1));
      const auto ref2 = reference::create(book, numbers.at(0), numbers.at(2));
      if(ref1 && ref2)
      {
        result.push_back(reference_range(ref1.value(), ref2.value()));
      }
      else
      {
        LOG_WARN(
          log_channel,
          "Invalid reference: book=[{}], chapter=[{}], verses=[{}-{}]",
          book,
          numbers.at(0),
          numbers.at(1),
          numbers.at(2));
      }
    }
    else {}
  }
  return result;
}

} // namespace bibstd::bible
