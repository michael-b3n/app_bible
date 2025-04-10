#pragma once

#include <atomic>
#include <format>
#include <type_traits>

namespace bibstd::util
{

///
/// Unique ID type.
///
template<typename Tag>
struct uid
{
  // Typedefs
  using tag_type = Tag;

  // Operators
  constexpr auto operator<=>(const uid&) const = default;

  // Variables
  std::uint64_t value;
};

///
/// Get new typesafe unique ID.
/// \return new typesafe unique ID
///
template<typename T>
auto new_uid() -> uid<T>
{
  static std::atomic_uint64_t counter{0};
  return {++counter};
}

} // namespace bibstd::util

///
///
template<typename T>
struct std::formatter<bibstd::util::uid<T>> : std::formatter<std::uint64_t>
{
  auto format(const bibstd::util::uid<T> id, std::format_context& ctx) const
  {
    return formatter<std::string>::format(std::format("{}", id.value), ctx);
  }
};
