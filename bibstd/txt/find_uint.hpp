#pragma once

#include <cassert>
#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>

namespace bibstd::txt
{

///
/// Find unsigned integer within string functor.
///
struct find_uint_t final
{
  ///
  /// Result of find_uint.
  /// \param value Unsigned integer value find in string_view
  /// \param offset Of substr_view from string_view parameter
  /// \param substr_view Substring of string_view containing unsigned integer
  ///
  struct result final
  {
    std::uint32_t value;
    std::size_t post_value_offset;
    constexpr auto operator<=>(const result&) const = default;
  };

  ///
  /// Functor operator.
  ///
  constexpr auto operator()(std::string_view text) const -> std::optional<result>;
};

///
/// Find unsigned integer within string view.
/// \param string_view String to search
/// \return result with value and meta information
///
constexpr find_uint_t find_uint;

///
///
constexpr auto find_uint_t::operator()(const std::string_view text) const -> std::optional<result>
{
  std::uint32_t value{0};
  const auto retval = std::from_chars(text.data(), text.end(), value);
  if(retval.ec != decltype(retval.ec){})
  {
    return std::nullopt;
  }
  const auto offset = std::ranges::distance(text.begin(), retval.ptr);
  assert(offset < 0);
  return result{.value = value, .post_value_offset = static_cast<std::size_t>(offset)};
}

} // namespace bibstd::txt
