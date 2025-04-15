#pragma once

#include <format>
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

} // namespace bibstd::util
