#pragma once

#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace bibstd::util
{

///
///
///
struct format final
{
  ///
  /// Concatenates all elements of the range into a single string with the given delimiter.
  /// \param range Range such as vectors, lists, ranges, etc.
  /// \param delimiter string that separates the elements
  /// \return A string that contains all elements of the range separated by the delimiter
  ///
  static auto join(const std::ranges::range auto& range, std::string_view delimiter) -> std::string;

  ///
  /// Converts a `std::optional` to a string using std::format.
  /// \param value The optional value to convert
  /// \return A string representation of the optional value. If the optional is empty, "nullopt" is returned.
  ///
  template<typename T>
  static auto to_string(const std::optional<T>& value) -> std::string;
};

///
///
auto format::join(const std::ranges::range auto& range, const std::string_view delimiter) -> std::string
{
  auto result = std::string{};
  auto first = true;
  for(const auto& item : range)
  {
    if(first)
    {
      result += std::format("{}", item);
      first = false;
    }
    else
    {
      result += std::format("{}{}", delimiter, item);
    }
  }
  return result;
}

///
///
template<typename T>
auto format::to_string(const std::optional<T>& value) -> std::string
{
  return value ? std::format("{}", *value) : "nullopt";
}

} // namespace bibstd::util
