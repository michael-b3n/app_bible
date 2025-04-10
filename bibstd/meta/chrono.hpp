#pragma once

#include <chrono>
#include <type_traits>

namespace bibstd::meta
{

///
/// std::chrono::duration type trait.
///
template<typename T>
struct is_duration : std::false_type
{};
template<typename T1, typename T2>
struct is_duration<std::chrono::duration<T1, T2>> : std::true_type
{};
template<typename T>
constexpr bool is_duration_v = is_duration<T>::value;

} // namespace bibstd::meta
