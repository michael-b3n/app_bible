#pragma once

#include "util/exception.hpp"
#include "util/string.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <concepts>
#include <cstdint>
#include <optional>
#include <ranges>
#include <string_view>
#include <type_traits>

namespace bibstd::txt
{

///
/// Chars struct containing special char lists.
///
class chars final
{
public: // Typedefs
  ///
  /// Char category.
  ///
  enum class category
  {
    letter,
    whitespace,
    line,
    fullstop,
    digit,
    other,
  };

  struct char_data final
  {
    category char_category{category::other};
    std::size_t char_size{0};
  };

public: // Constants
  ///
  /// All alphabet letter chars in an array.
  ///
  static constexpr auto letters = std::array{
    std::string_view("A"), std::string_view("a"), std::string_view("B"), std::string_view("b"), std::string_view("C"),
    std::string_view("c"), std::string_view("D"), std::string_view("d"), std::string_view("E"), std::string_view("e"),
    std::string_view("F"), std::string_view("f"), std::string_view("G"), std::string_view("g"), std::string_view("H"),
    std::string_view("h"), std::string_view("I"), std::string_view("i"), std::string_view("J"), std::string_view("j"),
    std::string_view("K"), std::string_view("k"), std::string_view("L"), std::string_view("l"), std::string_view("M"),
    std::string_view("m"), std::string_view("N"), std::string_view("n"), std::string_view("O"), std::string_view("o"),
    std::string_view("P"), std::string_view("p"), std::string_view("Q"), std::string_view("q"), std::string_view("R"),
    std::string_view("r"), std::string_view("S"), std::string_view("s"), std::string_view("T"), std::string_view("t"),
    std::string_view("U"), std::string_view("u"), std::string_view("V"), std::string_view("v"), std::string_view("W"),
    std::string_view("w"), std::string_view("X"), std::string_view("x"), std::string_view("Y"), std::string_view("y"),
    std::string_view("Z"), std::string_view("z"), std::string_view("Ä"), std::string_view("ä"), std::string_view("Ö"),
    std::string_view("ö"), std::string_view("Ü"), std::string_view("ü"),
  };

  ///
  /// All whitespace chars in an array.
  ///
  static constexpr auto whitespaces = std::array{
    std::string_view("\x20"),         // space
    std::string_view("\x09"),         // character tabulation
    std::string_view("\x09"),         // character tabulation
    std::string_view("\x0A"),         // line feed
    std::string_view("\x0B"),         // line tabulation
    std::string_view("\x0C"),         // form feed
    std::string_view("\x0D"),         // carriage return
    std::string_view("\x20"),         // space
    std::string_view("\xC2\x85"),     // next line
    std::string_view("\xC2\xA0"),     // no-break space
    std::string_view("\xE1\x9A\x80"), // ogham space mark
    std::string_view("\xE1\xA0\x8E"), // mongolian vowel separator
    std::string_view("\xE2\x80\x80"), // en quad
    std::string_view("\xE2\x80\x81"), // em quad
    std::string_view("\xE2\x80\x82"), // en space
    std::string_view("\xE2\x80\x83"), // em space
    std::string_view("\xE2\x80\x84"), // three-per-em space
    std::string_view("\xE2\x80\x85"), // four-per-em space
    std::string_view("\xE2\x80\x86"), // six-per-em space
    std::string_view("\xE2\x80\x87"), // figure space
    std::string_view("\xE2\x80\x88"), // punctuation space
    std::string_view("\xE2\x80\x89"), // thin space
    std::string_view("\xE2\x80\x8A"), // hair space
    std::string_view("\xE2\x80\x8B"), // zero width space
    std::string_view("\xE2\x80\x8C"), // zero width non-joiner
    std::string_view("\xE2\x80\x8D"), // zero width joiner
    std::string_view("\xE2\x80\xA8"), // line separator
    std::string_view("\xE2\x80\xA9"), // paragraph separator
    std::string_view("\xE2\x80\xAF"), // narrow no-break space
    std::string_view("\xE2\x81\x9F"), // medium mathematical space
    std::string_view("\xE2\x81\xA0"), // word joiner
    std::string_view("\xE3\x80\x80"), // ideographic space
    std::string_view("\xEF\xBB\xBF"), // zero width non-breaking space
  };

  ///
  /// All line chars in an array.
  ///
  static constexpr auto lines = std::array{
    std::string_view("\x2D"),         // hyphen-minus
    std::string_view("\xE2\x80\x90"), // hyphen
    std::string_view("\xE2\x80\x91"), // non-breaking hyphen
    std::string_view("\xE2\x80\x92"), // figure dash
    std::string_view("\xE2\x80\x93"), // en dash
    std::string_view("\xE2\x80\x94"), // em dash
    std::string_view("\xE2\x80\x95"), // horizontal bar
    std::string_view("\xEF\xB9\x98"), // small em dash
    std::string_view("\xEF\xB9\xA3"), // small hyphen-minus
    std::string_view("\xEF\xBC\x8D"), // fullwidth hyphen-minus
  };

  ///
  /// Checks if char in string at given index is of provided chars category.
  /// \param string_view String view
  /// \param index to check the character
  /// \param char_category Category of char that shall be checked
  /// \return optional string view of the char or std::nullopt if category does not match
  ///
  static constexpr auto is_char(std::string_view string_view, std::size_t index, category char_category)
    -> std::optional<std::string_view>;

  ///
  /// Check if char sequence is contained in a list of string_views.
  /// \tparam T List type with a value type which is equality comparable with std::string_view
  /// \param chars List of string_views comparable types
  /// \param string_view String view that shall be checked
  /// \param index Index in string view to check for
  /// \return optional string_view of the char or std::nullopt if char is not in list
  ///
  template<typename T>
    requires std::equality_comparable_with<typename T::value_type, std::string_view>
  static constexpr auto contains(const T& chars, std::string_view string_view, std::size_t index)
    -> std::optional<std::string_view>;

  ///
  /// Determines the category of the char in string at given index.
  /// \param string_view String view
  /// \param index to check the character
  /// \return category of the char
  ///
  static constexpr auto char_info(std::string_view string_view, std::size_t index) -> std::optional<char_data>;

  ///
  /// Calls function for each char in string_view. The char is classified to a category and passed to the function.
  /// \param string_view String view to iterate over
  /// \param function Function to call for each char of specified category
  ///
  template<typename Function>
    requires(std::is_invocable_v<Function, const std::string_view, const std::size_t, const category>)
  static constexpr auto for_each_char(std::string_view string_view, Function&& function) -> void;

  ///
  /// Calls function for each char in string_view. The char is classified to a category and passed to the function.
  /// \param string_view String view to iterate over
  /// \param function Function to call for each char of specified category
  ///
  template<typename Function>
    requires(std::is_invocable_r_v<bool, Function, const std::string_view, const std::size_t, const category>)
  static constexpr auto for_each_char_while(std::string_view string_view, Function&& function) -> void;

private: // Implementation
  static constexpr auto is_equal_impl(std::string_view string_view, std::size_t index, std::string_view character)
    -> std::optional<std::string_view>;
  static constexpr auto is_digit_impl(std::string_view string_view, std::size_t index) -> std::optional<std::string_view>;
};

///
///
constexpr auto chars::is_char(const std::string_view string_view, const std::size_t index, const category char_category)
  -> std::optional<std::string_view>
{
  if(index < string_view.size())
  {
    switch(char_category)
    {
    case category::letter: return contains(letters, string_view, index);
    case category::whitespace: return contains(whitespaces, string_view, index);
    case category::line: return contains(lines, string_view, index);
    case category::fullstop: return is_equal_impl(string_view, index, ".");
    case category::digit: return is_digit_impl(string_view, index);
    case category::other: return string_view.substr(index, 1);
    default: return std::nullopt;
    }
  }
  else
  {
    return std::nullopt;
  }
}

///
///
template<typename T>
  requires std::equality_comparable_with<typename T::value_type, std::string_view>
constexpr auto chars::contains(const T& chars, const std::string_view string_view, const std::size_t index)
  -> std::optional<std::string_view>
{
  if(index < string_view.size())
  {
    const auto string_size = string_view.size();
    const auto iter = std::ranges::find_if(
      chars,
      [&](const auto& e)
      {
        const auto size = std::min(e.size(), string_size - index);
        return util::make_substring_view(string_view, index, size) == e;
      }
    );
    return iter != std::ranges::cend(chars) ? std::make_optional(*iter) : std::nullopt;
  }
  else
  {
    return std::nullopt;
  }
}

///
///
constexpr auto chars::char_info(std::string_view string_view, std::size_t index) -> std::optional<char_data>
{
  // clang-format off
  if(const auto result = is_char(string_view, index, category::letter); result) { return char_data{category::letter, result->size()}; }
  if(const auto result = is_char(string_view, index, category::whitespace); result) { return char_data{category::whitespace, result->size()}; }
  if(const auto result = is_char(string_view, index, category::line); result) { return char_data{category::line, result->size()}; }
  if(const auto result = is_char(string_view, index, category::fullstop); result) { return char_data{category::fullstop, result->size()}; }
  if(const auto result = is_char(string_view, index, category::digit); result) { return char_data{category::digit, result->size()}; }
  if(const auto result = is_char(string_view, index, category::other); result) { return char_data{category::other, result->size()}; }
  // clang-format on
  return std::nullopt;
}

///
///
template<typename Function>
  requires(std::is_invocable_v<Function, const std::string_view, const std::size_t, const chars::category>)
constexpr auto chars::for_each_char(const std::string_view string_view, Function&& function) -> void
{
  auto counter = std::size_t{0};
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, string_view.size()) |
      std::views::take_while([&](const auto i) { return counter < string_view.size(); }),
    [&]([[maybe_unused]] const auto)
    {
      const auto data = char_info(string_view, counter);
      if(data)
      {
        function(string_view.substr(counter, data->char_size), counter, data->char_category);
        counter += data->char_size;
      }
      else
      {
        LOG_ERROR("error getting char info: char=\'{}\'", string_view.at(counter));
        ++counter;
      }
    }
  );
}

///
///
template<typename Function>
  requires(std::is_invocable_r_v<bool, Function, const std::string_view, const std::size_t, const chars::category>)
constexpr auto chars::for_each_char_while(const std::string_view string_view, Function&& function) -> void
{
  auto counter = std::size_t{0};
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, string_view.size()) |
      std::views::take_while([&](const auto i) { return counter < string_view.size(); }),
    [&]([[maybe_unused]] const auto)
    {
      const auto data = char_info(string_view, counter);
      if(data)
      {
        if(function(string_view.substr(counter, data->char_size), counter, data->char_category))
        {
          counter += data->char_size;
        }
        else
        {
          counter = string_view.size();
        }
      }
      else
      {
        LOG_ERROR("error getting char info: char=\'{}\'", string_view.at(counter));
        ++counter;
      }
    }
  );
}

///
///
constexpr auto chars::is_equal_impl(const std::string_view string_view, const std::size_t index, const std::string_view c)
  -> std::optional<std::string_view>
{
  if(index + c.size() > string_view.size())
  {
    return std::nullopt;
  }
  const auto substr = string_view.substr(index, c.size());
  return substr == c ? std::make_optional(substr) : std::nullopt;
}

///
///
constexpr auto chars::is_digit_impl(const std::string_view string_view, const std::size_t index)
  -> std::optional<std::string_view>
{
  return std::isdigit(static_cast<unsigned char>(string_view.at(index))) ? std::make_optional(string_view.substr(index, 1))
                                                                         : std::nullopt;
}

} // namespace bibstd::txt
