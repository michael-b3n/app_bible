#pragma once

#include "math/arithmetic.hpp"
#include <magic_enum/magic_enum.hpp>

#include <concepts>
#include <string>
#include <type_traits>

namespace bibstd::util
{

///
/// Concept to check if E is enum.
///
template<typename E>
concept enum_type = std::is_enum_v<E>;

///
/// Converts enum to integral.
/// \tparam E enum type
/// \param e enum value
/// \return integral value corresponding to enum value
///
template<enum_type E>
constexpr auto to_integral(const E e) -> std::underlying_type_t<E>
{
  return static_cast<std::underlying_type_t<E>>(e);
}

///
/// Converts enum to string.
/// \tparam E enum type
/// \param e enum value
/// \return string_view name corresponding to enum value
///
template<enum_type E>
constexpr auto to_string_view(const E e) -> std::string_view
{
  return magic_enum::enum_name(e);
}

///
/// Converts integral to enum.
/// \tparam E enum type
/// \param integral enums underlying type value
/// \return enum type value
///
template<enum_type E>
constexpr auto to_enum(const std::underlying_type_t<E> integral) -> E
{
  return static_cast<E>(integral);
}

///
/// Converts string view enum name to enum.
/// \tparam E enum type
/// \param name name of enum value as string view
/// \return optional enum type value, std::nullopt of name does not correspond to any enum value in specified enum type
///
template<enum_type E>
constexpr auto to_enum(const std::string_view name) -> std::optional<E>
{
  const auto value = magic_enum::enum_cast<E>(name);
  return value ? *value : std::optional<E>{std::nullopt};
}

///
/// Get next enum value.
/// \tparam E enum type
/// \param e enum value
/// \return next enum value of param e
///
template<enum_type E>
constexpr auto next(const E e) -> E
{
  const auto value = math::arithmetic::add(to_integral(e), std::underlying_type_t<E>(1));
  if(!value.has_value())
  {
    THROW_EXCEPTION(std::runtime_error("Invalid next enum value"));
  }
  return to_enum<E>(*value);
}

///
/// Get previous enum value.
/// \tparam E enum type
/// \param e enum value
/// \return previous enum value of param e
///
template<enum_type E>
constexpr auto prev(const E e) -> E
{
  const auto value = math::arithmetic::subtract(to_integral(e), std::underlying_type_t<E>(1));
  if(!value.has_value())
  {
    THROW_EXCEPTION(std::runtime_error("Invalid previous enum value"));
  }
  return to_enum<E>(*value);
}

///
/// Checks if enum value of fenced enum type is valid.
/// \tparam E fenced enum type
/// \param e fenced enum value
/// \param true if between begin and end, false otherwise
///
template<enum_type E>
constexpr auto valid(E e) -> bool
{
  constexpr std::size_t enum_count = magic_enum::enum_count<E>();
  return magic_enum::enum_cast<E>(to_integral(e)).has_value();
}

} // namespace bibstd::util
