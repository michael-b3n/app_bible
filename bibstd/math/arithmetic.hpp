#pragma once

#include "math/sign.hpp"
#include "meta/type_traits.hpp"
#include "util/exception.hpp"

#include <concepts>
#include <expected>
#include <limits>
#include <type_traits>

namespace bibstd::math
{

///
/// Arithmetic type concept.
///
template<typename T>
concept arithmetic_type = std::is_arithmetic_v<T>;

///
/// Arithmetics guard for save math operations guaranteeing over- and underflow protection.
///
struct arithmetic final
{
  // Typedefs
  ///
  /// Arithmetics error code.
  ///
  enum class error_code
  {
    overflow,
    underflow
  };
  template<arithmetic_type T>
  using expected_type = std::expected<T, error_code>;

  // Static functions
  ///
  /// Add two values of an arithmetic type `T`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first + second`
  ///
  template<arithmetic_type T>
  static auto add(T first, T second) -> expected_type<T>;

  ///
  /// Add two values of type `expected_type<T>`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first + second`
  ///
  template<arithmetic_type T>
  static auto add(expected_type<T> first, expected_type<T> second) -> expected_type<T>;

  ///
  /// Subtract two values of an arithmetic type `T`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first - second`
  ///
  template<arithmetic_type T>
  static auto subtract(T first, T second) -> expected_type<T>;

  ///
  /// Subtract two values of type `expected_type<T>`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first - second`
  ///
  template<arithmetic_type T>
  static auto subtract(expected_type<T> first, expected_type<T> second) -> expected_type<T>;

  ///
  /// Multiply two values of an arithmetic type `T`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first * second`
  ///
  template<arithmetic_type T>
  static auto multiply(T first, T second) -> expected_type<T>;

  ///
  /// Multiply two values of type `expected_type<T>`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first * second`
  ///
  template<arithmetic_type T>
  static auto multiply(expected_type<T> first, expected_type<T> second) -> expected_type<T>;

  ///
  /// Divide two values of an arithmetic type `T`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first / second`
  ///
  template<arithmetic_type T>
  static auto divide(T first, T second) -> expected_type<T>;

  ///
  /// Divide two values of type `expected_type<T>`.
  /// \param first First value
  /// \param second Second value
  /// \return result of `first / second`
  ///
  template<arithmetic_type T>
  static auto divide(expected_type<T> first, expected_type<T> second) -> expected_type<T>;

  ///
  /// Clamp to lowest or max value depending on under- or overflow.
  /// \param value Expected value that shall be clamped
  /// \return clamped value
  ///
  template<arithmetic_type T>
  static auto clamp(expected_type<T> value) -> T;
};

namespace detail
{

template<typename T>
struct arithmetic_operation;

template<signed_type T>
struct arithmetic_operation<T>
{
  static auto add(T first, T second) -> arithmetic::expected_type<T>;
  static auto subtract(T first, T second) -> arithmetic::expected_type<T>;
  static auto multiply(T first, T second) -> arithmetic::expected_type<T>;
  static auto divide(T first, T second) -> arithmetic::expected_type<T>;
};

template<unsigned_type T>
struct arithmetic_operation<T>
{
  static auto add(T first, T second) -> arithmetic::expected_type<T>;
  static auto subtract(T first, T second) -> arithmetic::expected_type<T>;
  static auto multiply(T first, T second) -> arithmetic::expected_type<T>;
  static auto divide(T first, T second) -> arithmetic::expected_type<T>;
};

} // namespace detail

///
///
template<arithmetic_type T>
auto arithmetic::add(const T first, const T second) -> expected_type<T>
{
  return detail::arithmetic_operation<T>::add(first, second);
}

///
///
template<arithmetic_type T>
auto arithmetic::add(const expected_type<T> first, const expected_type<T> second) -> expected_type<T>
{
  using op = detail::arithmetic_operation<T>;
  return first.and_then([&](const T first_v) { return second.and_then([&](const T second_v) { return op::add(first_v, second_v); }); });
}

///
///
template<arithmetic_type T>
auto arithmetic::subtract(const T first, const T second) -> expected_type<T>
{
  return detail::arithmetic_operation<T>::subtract(first, second);
}

///
///
template<arithmetic_type T>
auto arithmetic::subtract(const expected_type<T> first, const expected_type<T> second) -> expected_type<T>
{
  using op = detail::arithmetic_operation<T>;
  return first.and_then([&](const T first_v) { return second.and_then([&](const T second_v) { return op::subtract(first_v, second_v); }); });
}

///
///
template<arithmetic_type T>
auto arithmetic::multiply(const T first, const T second) -> expected_type<T>
{
  return detail::arithmetic_operation<T>::multiply(first, second);
}

///
///
template<arithmetic_type T>
auto arithmetic::multiply(const expected_type<T> first, const expected_type<T> second) -> expected_type<T>
{
  using op = detail::arithmetic_operation<T>;
  return first.and_then([&](const T first_v) { return second.and_then([&](const T second_v) { return op::multiply(first_v, second_v); }); });
}

///
///
template<arithmetic_type T>
auto arithmetic::divide(const T first, const T second) -> expected_type<T>
{
  return detail::arithmetic_operation<T>::divide(first, second);
}

///
///
template<arithmetic_type T>
auto arithmetic::divide(const expected_type<T> first, const expected_type<T> second) -> expected_type<T>
{
  using op = detail::arithmetic_operation<T>;
  return first.and_then([&](const T first_v) { return second.and_then([&](const T second_v) { return op::divide(first_v, second_v); }); });
}

///
///
template<arithmetic_type T>
auto arithmetic::clamp(expected_type<T> value) -> T
{
  if(value.has_value())
  {
    return value.value();
  }
  else
  {
    switch(value.error())
    {
    case error_code::overflow: return std::numeric_limits<T>::max();
    case error_code::underflow: return std::numeric_limits<T>::lowest();
    default: THROW_EXCEPTION(util::exception("Unknown error code")); return T{0};
    }
  }
}

namespace detail
{

///
///
template<signed_type T>
auto arithmetic_operation<T>::add(T first, T second) -> arithmetic::expected_type<T>
{
  const auto second_sign = sign(second);
  const auto overflow = second_sign == sign_value::positive && first > std::numeric_limits<T>::max() - second;
  const auto underflow = second_sign == sign_value::negative && first < std::numeric_limits<T>::lowest() - second;
  if(overflow)
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else if(underflow)
  {
    return std::unexpected{arithmetic::error_code::underflow};
  }
  else
  {
    return first + second;
  }
}

///
///
template<signed_type T>
auto arithmetic_operation<T>::subtract(T first, T second) -> arithmetic::expected_type<T>
{
  const auto second_sign = sign(second);
  const auto overflow = second_sign == sign_value::negative && first > std::numeric_limits<T>::max() + second;
  const auto underflow = second_sign == sign_value::positive && first < std::numeric_limits<T>::lowest() + second;
  if(overflow)
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else if(underflow)
  {
    return std::unexpected{arithmetic::error_code::underflow};
  }
  else
  {
    return first - second;
  }
}

///
///
template<signed_type T>
auto arithmetic_operation<T>::multiply(T first, T second) -> arithmetic::expected_type<T>
{
  if(second == T{0} || first == T{0})
  {
    return T{0};
  }
  const auto multiplied_sign = sign(first) * sign(second);
  const auto overflow = multiplied_sign == sign_value::positive && std::abs(first) > std::numeric_limits<T>::max() / std::abs(second);
  const auto underflow = multiplied_sign == sign_value::negative && T{-1} * std::abs(first) < std::numeric_limits<T>::lowest() / std::abs(second);
  if(overflow)
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else if(underflow)
  {
    return std::unexpected{arithmetic::error_code::underflow};
  }
  else
  {
    return first * second;
  }
}

///
///
template<signed_type T>
auto arithmetic_operation<T>::divide(T first, T second) -> arithmetic::expected_type<T>
{
  auto overflow = false;
  auto underflow = false;
  if(first == T{0})
  {
    return T{0};
  }
  else if(second == T{0})
  {
    const auto first_sign = sign(first);
    sign(first) == sign_value::positive ? overflow = true : underflow = true;
  }
  else if(second < T{1})
  {
    const auto multiplied_sign = sign(first) * sign(second);
    overflow = multiplied_sign == sign_value::positive && std::abs(first) > std::numeric_limits<T>::max() * std::abs(second);
    underflow = multiplied_sign == sign_value::negative && T{-1} * std::abs(first) < std::numeric_limits<T>::lowest() * std::abs(second);
  }
  if(overflow)
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else if(underflow)
  {
    return std::unexpected{arithmetic::error_code::underflow};
  }
  else
  {
    return first / second;
  }
}

///
///
template<unsigned_type T>
auto arithmetic_operation<T>::add(T first, T second) -> arithmetic::expected_type<T>
{
  if(first > std::numeric_limits<T>::max() - second)
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else
  {
    return first + second;
  }
}

///
///
template<unsigned_type T>
auto arithmetic_operation<T>::subtract(T first, T second) -> arithmetic::expected_type<T>
{
  if(first < std::numeric_limits<T>::lowest() + second)
  {
    return std::unexpected{arithmetic::error_code::underflow};
  }
  else
  {
    return first - second;
  }
}

///
///
template<unsigned_type T>
auto arithmetic_operation<T>::multiply(T first, T second) -> arithmetic::expected_type<T>
{
  if(second == T{0} || first == T{0})
  {
    return T{0};
  }
  else if(first > std::numeric_limits<T>::max() / second)
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else
  {
    return first * second;
  }
}

///
///
template<unsigned_type T>
auto arithmetic_operation<T>::divide(T first, T second) -> arithmetic::expected_type<T>
{
  if(first == T{0})
  {
    return T{0};
  }
  else if(second == T{0})
  {
    return std::unexpected{arithmetic::error_code::overflow};
  }
  else
  {
    return first / second;
  }
}

} // namespace detail
} // namespace bibstd::math
