#pragma once

#include <concepts>
#include <type_traits>

namespace bibstd::util
{

///
/// Concept to check if E is enum.
///
template<typename E>
concept enum_type = std::is_enum_v<E>;

///
/// Concept to check if E has a defined begin and end.
///
template<typename E>
concept constrained_enum = enum_type<E>&&
  requires(E)
{
  E::BEGIN;
  E::END;
};

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
/// Get next enum value.
/// \tparam E enum type
/// \param e enum value
/// \return next enum value of param e
///
template<enum_type E>
constexpr auto next(const E e) -> E
{
  return to_enum<E>(to_integral(e) + 1);
}

///
/// Checks if enum value of fenced enum type is valid.
/// \tparam E fenced enum type
/// \param e fenced enum value
/// \param true if between begin and end, false otherwise
///
template<constrained_enum E>
constexpr auto valid(E e) -> bool
{
  return E::BEGIN <= e && e < E::END;
}

///
/// Convert a constrained enum to an array.
/// \tparam E constrained enum having E::BEGIN and E::END
/// \return std::array with all enum values from E::BEGIN to E::END
///
template<constrained_enum E>
constexpr auto enum_to_array() -> auto
{
  static_assert(E::BEGIN < E::END);
  static constexpr auto size = util::to_integral(E::END) - util::to_integral(E::BEGIN);
  static constexpr auto offset = util::to_integral(E::BEGIN);
  constexpr auto to_value = [&]<std::underlying_type_t<E> I>() { return util::to_enum<E>(I + offset); };
  return [&]<std::underlying_type_t<E>... I>(std::integer_sequence<std::underlying_type_t<E>, I...>)
  { return std::array{to_value.template operator()<I>()...}; }(std::make_integer_sequence<std::underlying_type_t<E>, size>{});
}

} // namespace bibstd::util
