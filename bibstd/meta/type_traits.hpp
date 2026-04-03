#pragma once

#include <optional>
#include <type_traits>

namespace bibstd::meta
{

///
/// Type trait to check if a type is templated.
///
template<typename T>
struct is_templated final : std::false_type
{};
template<template<typename...> typename T, typename... Args>
struct is_templated<T<Args...>> final : std::true_type
{};
template<typename T>
inline constexpr auto is_templated_v = is_templated<std::remove_cvref_t<T>>::value;

///
/// Type trait that removes the optional wrapper from a type if it is an optional, otherwise returns the type itself.
///
template<typename T>
struct remove_optional final : std::false_type
{
  using type = T;
};
template<typename T>
struct remove_optional<std::optional<T>> final : std::true_type
{
  using type = T;
};
template<typename T>
using remove_optional_t = typename remove_optional<T>::type;

///
/// Type trait to check if a type is an optional.
///
template<typename T>
inline constexpr bool is_optional_v = remove_optional<T>::value;

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
inline constexpr bool are_same_v = are_same<T...>::value;

} // namespace bibstd::meta
