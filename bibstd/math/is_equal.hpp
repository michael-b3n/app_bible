#pragma once

#include "math/sign.hpp"

#include <cmath>
#include <concepts>
#include <limits>

namespace bibstd::math
{

///
/// Check if a floating point value is almost equal to another floating point value.
/// \tparam T Arithmetic floating point type
/// \param v1 First value
/// \param v2 Second value
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
/// \tparam T1 Arithmetic integral type
/// \tparam T2 Arithmetic integral type
/// \param v1 First value
/// \param v2 Second value
/// \return true, if first value is equal to second value
///
template<std::integral T1, std::integral T2>
  requires std::equality_comparable_with<T1, T2>
constexpr auto is_equal(
  const T1 v1,
  const T2 v2,
  [[maybe_unused]] const std::common_type_t<T1, T2> epsilon = static_cast<std::common_type_t<T1, T2>>(0)
) -> bool
{
  if constexpr(std::is_signed_v<T1> != std::is_signed_v<T2>)
  {
    if constexpr(std::is_signed_v<T1>)
    {
      return sign(v1) == sign_value::negative ? false : static_cast<std::make_unsigned_t<T1>>(v1) == v2;
    }
    else
    {
      return sign(v2) == sign_value::negative ? false : static_cast<std::make_unsigned_t<T2>>(v2) == v1;
    }
  }
  else
  {
    return v1 == v2;
  }
}

///
/// Check if a integral value is equal to another equality comparable value.
/// \tparam T Arithmetic integral type
/// \tparam T1 Equality comparable to T type
/// \param v1 First value
/// \param v2 Second value
/// \return true, if first value is equal to second value
///
template<std::integral T, typename T1>
  requires(std::equality_comparable_with<T1, T> && !std::integral<T1>)
constexpr auto is_equal(const T v1, const T1 v2, [[maybe_unused]] const T epsilon = static_cast<T>(0)) -> bool
{
  return v1 == v2;
}

///
/// Check if a integral value is equal to another equality comparable value.
/// \tparam T Arithmetic integral type
/// \tparam T1 Equality comparable to T type
/// \param v1 First value
/// \param v2 Second value
/// \return true, if first value is equal to second value
///
template<std::integral T, typename T1>
  requires(std::equality_comparable_with<T1, T> && !std::integral<T1>)
constexpr auto is_equal(const T1 v1, const T v2, [[maybe_unused]] const T epsilon = static_cast<T>(0)) -> bool
{
  return v1 == v2;
}

} // namespace bibstd::math
