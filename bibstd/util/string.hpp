#pragma once

#include <array>
#include <cassert>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace bibstd::util
{

///
/// Concept to check if T is string view type.
///
template<typename T>
concept string_view_type = std::is_convertible_v<T, std::string_view>;

///
/// Create string view array from string literal args.
/// \param ...args of string literal types
/// \return string view array
///
template<string_view_type... S>
constexpr auto to_array(S&&... args) -> std::array<std::string_view, sizeof...(S)>
{
  return std::array{std::string_view(std::forward<decltype(args)>(args))...};
}

///
/// Check if string_view starts with prefix.
/// \param string_view String view to check
/// \param prefix Prefix of string_view to check
/// \return true if string_view starts with prefix, false otherwise
///
constexpr auto starts_with(const std::string_view string_view, const std::string_view prefix) -> bool
{
  return prefix.size() <= string_view.size() && std::equal(std::cbegin(prefix), std::cend(prefix), std::cbegin(string_view));
}

///
/// Check if string_view starts with prefix.
/// \param string_view String view to check
/// \param prefix_char Prefix char of string_view to check
/// \return true if string_view starts with char, false otherwise
///
constexpr auto starts_with(const std::string_view string_view, const char prefix) -> bool
{
  return !string_view.empty() && string_view.front() == prefix;
}

} // namespace bibstd::util
