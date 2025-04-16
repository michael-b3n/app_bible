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
class uid final
{
  // Friends
  template<typename T>
  friend struct std::formatter;

public: // Typedefs
  using tag_type = Tag;

public: // Static helpers
  ///
  /// Get new typesafe unique ID.
  /// \return new typesafe unique ID
  ///
  static auto new_uid() -> uid<tag_type>;

public: // Constructor
  uid();

public: // Structors
  constexpr auto operator<=>(const uid&) const = default;

private: // Structors
  constexpr uid(std::uint64_t value);

private: // Variables
  std::uint64_t value_;
};

///
/// Get new typesafe unique ID.
/// \return new typesafe unique ID
///
template<typename Tag>
auto uid<Tag>::new_uid() -> uid<tag_type>
{
  static std::atomic_uint64_t counter{0};
  return {++counter};
}

///
///
template<typename Tag>
uid<Tag>::uid()
  : uid{new_uid()}
{
}

///
///
template<typename Tag>
constexpr uid<Tag>::uid(std::uint64_t value)
  : value_{value}
{
}

} // namespace bibstd::util

///
///
template<typename T>
struct std::formatter<bibstd::util::uid<T>> : std::formatter<std::uint64_t>
{
  auto format(const bibstd::util::uid<T> id, std::format_context& ctx) const
  {
    return formatter<std::string>::format(std::format("{}", id.value_), ctx);
  }
};
