#pragma once

#include "bibstd/util/ranges.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace bibstd::util::string
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
constexpr auto to_string_view_array(S&&... args) -> std::array<std::string_view, sizeof...(S)>
{
  return std::array{std::string_view(std::forward<decltype(args)>(args))...};
}

///
/// Split string_view by delimiter and return vector of string parts.
/// \param data String view to split
/// \param delimiter Delimiter to split by
/// \return vector of string parts
///
constexpr auto split(const std::string_view data, const auto delimiter) -> std::vector<std::string>
  requires(std::convertible_to<decltype(delimiter), std::string_view> || std::convertible_to<decltype(delimiter), char>)
{
  const auto delimiter_size = [&]() -> std::size_t
  {
    if constexpr(std::convertible_to<decltype(delimiter), std::string_view>)
    {
      return std::string_view(delimiter).size();
    }
    else
    {
      return 1;
    }
  }();
  auto result = std::vector<std::string>{};
  auto current_index = decltype(data.size()){0};

  std::ranges::for_each(
    util::ranges::index_view(data) | std::views::take_while([&](const auto i) { return current_index < data.size(); }),
    [&]([[maybe_unused]] const auto)
    {
      const auto pos = data.substr(current_index).find(delimiter);
      result.emplace_back(data.substr(current_index, pos));
      current_index = pos != std::string_view::npos ? current_index + pos + delimiter_size : data.size();
    }
  );
  return result;
}

///
/// Join string parts with delimiter and return the joined string.
/// \param parts Vector of string parts to join
/// \param delimiter Delimiter to join with
/// \return Joined string
///
constexpr auto join(const std::ranges::range auto& parts, const auto delimiter) -> std::string
  requires(
    std::convertible_to<std::ranges::range_value_t<decltype(parts)>, std::string_view> &&
    (std::convertible_to<decltype(delimiter), std::string_view> || std::convertible_to<decltype(delimiter), char>)
  )
{
  if(parts.empty())
  {
    return {};
  }
  return std::ranges::fold_left(
    std::ranges::next(std::ranges::cbegin(parts)),
    std::ranges::cend(parts),
    parts.front(),
    [&](const auto& a, const auto& b) { return std::format("{}{}{}", a, delimiter, b); }
  );
}

///
/// Check if string_view starts with prefix.
/// \param data String view to check for begin
/// \param prefix Prefix of string_view to check
/// \return true if string_view starts with prefix, false otherwise
///
constexpr auto starts_with(const std::string_view data, const std::string_view prefix) -> bool
{
  return prefix.size() <= data.size() && std::equal(std::cbegin(prefix), std::cend(prefix), std::cbegin(data));
}

///
/// Check if string_view ends with postfix.
/// \param data String view to check for ending
/// \param postfix Postfix of string_view to check
/// \return true if string_view ends with postfix, false otherwise
///
constexpr auto ends_with(const std::string_view data, const std::string_view postfix) -> bool
{
  return postfix.size() <= data.size() &&
         std::equal(std::cbegin(postfix), std::cend(postfix), std::prev(std::cend(data), postfix.size()));
}

///
/// Check if string_view starts with prefix.
/// \param data String view to check for begin
/// \param prefix_char Prefix char to check
/// \return true if string_view starts with char, false otherwise
///
constexpr auto starts_with(const std::string_view data, const char prefix_char) -> bool
{
  return !data.empty() && data.front() == prefix_char;
}

///
/// Check if string_view ends with postfix.
/// \param data String view to check for ending
/// \param postfix_char Postfix char to check
/// \return true if string_view ends with char, false otherwise
///
constexpr auto ends_with(const std::string_view data, const char postfix_char) -> bool
{
  return !data.empty() && data.back() == postfix_char;
}

} // namespace bibstd::util::string
