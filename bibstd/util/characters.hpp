#pragma once

#include "util/string_view.hpp"

#include <array>
#include <cassert>
#include <string>
#include <string_view>

namespace bibstd::util
{

///
/// \brief Common definitions and helpers for strings.
///
struct characters final
{
  ///
  /// @brief All whitespaces in utf-8 encoding.
  ///
  static constexpr auto whitespaces = std::array{
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
  using whitespaces_type = decltype(whitespaces);

  ///
  /// \brief Checks if char in string at given index is of whitespace type.
  /// \param string_view to create sub string view from
  /// \param index to check the character
  /// \return iterator to element of whitespaces
  ///
  static constexpr auto is_whitespace(const std::string_view& string_view, std::size_t index) noexcept
    -> whitespaces_type::const_iterator;

  // auto next_no_whitespace_idx(const std::string_view& string, std::size_t index) -> std::size_t
  // {
  //   if(index == string.size())
  //   {
  //     return index;
  //   }
  //   assert(index < string.size());

  //   const auto new_idx = skip_if_white_space(string, index);
  //   if(new_idx != index)
  //   {
  //     return next_no_whitespace_idx(string, new_idx);
  //   }
  //   return index;
  // }
};

///
///
constexpr auto characters::is_whitespace(const std::string_view& string_view, std::size_t index) noexcept
  -> whitespaces_type::const_iterator
{
  assert(index < string_view.size());
  const auto string_size = string_view.size();
  const auto iter = std::ranges::find_if(
    whitespaces,
    [&](const auto& e)
    {
      const auto size = std::min(e.size(), string_size - index);
      return create_substring_view(string_view, index, size) == e;
    });
  return iter;
}

} // namespace bibstd::util
