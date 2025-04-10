#pragma once

#include <type_traits>

namespace bibstd::meta
{

///
/// Are same type trait. Checks variadic pack on same type.
///
template<typename... T>
struct are_same : std::true_type
{};
template<typename T1, typename T2, typename... T>
struct are_same<T1, T2, T...>
{
  static constexpr bool value = std::is_same_v<T1, T2> && are_same<T2, T...>::value;
};
template<typename... T>
constexpr bool are_same_v = are_same<T...>::value;

} // namespace bibstd::meta
