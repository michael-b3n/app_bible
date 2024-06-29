#pragma once

#include <array>
#include <expected>
#include <map>
#include <type_traits>
#include <utility>

namespace bibstd::meta
{

///
/// std::expected type trait.
///
template<typename T>
struct is_expected : std::false_type
{};
template<typename T1, typename T2>
struct is_expected<std::expected<T1, T2>> : std::true_type
{};
template<typename T>
constexpr bool is_expected_v = is_expected<T>::value;

template<typename T>
concept expected_type = is_expected_v<T>;

///
/// std::pair type trait.
///
template<typename T>
struct is_pair : std::false_type
{};
template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type
{};
template<typename T>
constexpr bool is_pair_v = is_pair<T>::value;

template<typename T>
concept pair_type = is_pair_v<T>;

///
/// std::array type trait.
///
template<typename T>
struct is_array : std::false_type
{};
template<typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type
{};
template<typename T>
constexpr bool is_array_v = is_array<T>::value;

template<typename T>
concept array_type = is_array_v<T>;

///
/// std::map type trait.
///
template<typename T>
struct is_map : std::false_type
{};
template<typename K, typename V>
struct is_map<std::map<K, V>> : std::true_type
{};
template<typename T>
constexpr bool is_map_v = is_map<T>::value;

template<typename T>
concept map_type = is_map_v<T>;

///
/// pack_type trait.
///
template<typename T>
struct is_pack : std::false_type
{};
template<template<typename...> typename T, typename... Args>
struct is_pack<T<Args...>> : std::true_type
{};
template<typename T>
constexpr bool is_pack_v = is_pack<T>::value;

template<typename T>
concept pack_type = is_pack_v<T>;

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
