#pragma once

#include <array>
#include <cassert>
#include <string>
#include <string_view>
#include <type_traits>

namespace bibstd::util
{

///
/// Concept to check if T is string view type.
///
template<typename T>
concept string_view_type = std::is_convertible_v<T, std::string_view>;

///
/// Convert string view to normal string.
/// \param string_view that shall be converted
/// \return string with content of string view
///
inline auto to_string(const std::string_view string_view) -> std::string
{
  return std::string{string_view.begin(), string_view.end()};
}

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
/// \brief Create sub view from string view.
/// \param string_view to create sub string view from
/// \param offset of string view
/// \param size size of substring view
/// \return created substring view
///
constexpr auto make_substring_view(const std::string_view& string_view, const std::size_t offset, const std::size_t size) -> std::string_view
{
  assert(offset + size <= string_view.size());
  const auto begin = std::cbegin(string_view);
  return std::string_view{begin + offset, begin + (offset + size)};
}

///
/// \brief Create sub view from string.
/// \param string to create sub string view from
/// \param offset of string
/// \param size size of substring view
/// \return created substring view
///
constexpr auto make_substring_view(const std::string& string, const std::size_t offset, const std::size_t size) -> std::string_view
{
  assert(offset + size <= string.size());
  const auto begin = std::cbegin(string);
  return std::string_view{begin + offset, begin + (offset + size)};
}

} // namespace bibstd::util
