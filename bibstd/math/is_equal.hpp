#pragma once

#include <cmath>
#include <concepts>
#include <limits>

namespace bibstd::math
{

///
/// Check if a floating point value is almost equal to another floating point value.
/// \tparam T Arithmetic floating point type
/// \param v1 First value
/// \param v1 Second value
/// \param epsilon Small value defining almost equal as `|v1 - v2| < epsilon`
/// \return true, if first value is almost equal to second value
///
template<std::floating_point T>
constexpr auto is_equal(const T v1, const T v2, const T epsilon = std::numeric_limits<T>::epsilon()) -> bool
{
  return std::abs(v1 - v2) < epsilon;
}

///
/// Check if a integral value is equal to another integral value.
/// \tparam T Arithmetic integral type
/// \param v1 First value
/// \param v1 Second value
/// \return true, if first value is equal to second value
///
template<std::integral T>
constexpr auto is_equal(const T v1, const T v2, [[maybe_unused]] const T epsilon = static_cast<T>(0)) -> bool
{
  return v1 == v2;
}

} // namespace bibstd::math
