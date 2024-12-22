#pragma once

#include "meta/type_traits.hpp"

#include <type_traits>
#include <utility>

namespace bibstd::meta
{
namespace detail
{

///
/// Index helper type.
///
template<std::size_t N>
struct index_t
{
  static constexpr std::size_t index = N;
};

} // namespace detail

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
/// Unpack all types in pack like type P into template T.
///
template<template<typename...> typename T, pack_type P>
struct unpack;

template<template<typename...> typename T, template<typename...> typename P, typename... Args>
struct unpack<T, P<Args...>>
{
  using type = T<Args...>;
};

///
/// Unpack method to unpack a pack P into a template type T<...>.
/// \tparam T<...> variadic templated type
/// \tparam P pack of types
/// \result type of T<P<0>, P<1>, ... P<n>>
///
template<template<typename...> class T, typename P>
using unpack_t = typename unpack<T, P>::type;

///
/// Add type to pack like type from the left side.
///
template<typename T, pack_type P>
struct add_to_pack;

template<typename T, template<typename...> typename P, typename... Args>
struct add_to_pack<T, P<Args...>>
{
  using type = P<T, Args...>;
};

///
/// Add to pack method to add T to a pack P from the left side.
/// \tparam T generic type
/// \tparam P pack of types
/// \result type of pack<T, P<0>, P<1>, ... P<n>>
///
template<typename T, typename P>
using add_to_pack_t = typename add_to_pack<T, P>::type;

///
/// Combine two pack types to one pack type. Pack type type is of first type.
///
template<pack_type T, pack_type P>
struct combine_pack;

template<template<typename...> typename P1, template<typename...> typename P2, typename... Ts, typename... Args>
struct combine_pack<P1<Ts...>, P2<Args...>>
{
  using type = P1<Ts..., Args...>;
};

///
/// Combine pack method to combine pack T with pack P. Type is of pack type T.
/// \tparam T pack of types
/// \tparam P pack of types
/// \result type of T<T<0>, T<1>, ... T<m>, P<0>, P<1>, ... P<n>>
///
template<typename T, typename P>
using combine_pack_t = typename combine_pack<T, P>::type;

///
/// Remove first type from pack.
///
template<pack_type P>
struct remove_from_pack;

template<template<typename...> typename P, typename T, typename... Args>
struct remove_from_pack<P<T, Args...>>
{
  using type = P<Args...>;
};

///
/// Remove from pack method to remove T from a pack P from the left side.
/// \tparam P pack of types as pack<P<0>, P<1>, ... P<n>>
/// \result type of pack<P<1>, ... P<n>>
///
template<typename P>
using remove_from_pack_t = typename remove_from_pack<P>::type;

///
/// Pack types into pack like type N times.
///
template<template<typename...> typename P, typename T, std::size_t N>
struct pack_n_types
{
private:
  template<pack_type P_, typename T_, std::size_t N_>
  struct pack_n_types_helper
  {
    using type = typename pack_n_types_helper<add_to_pack_t<T_, P_>, T_, N_ - 1>::type;
  };

  template<pack_type P_, typename T_>
  struct pack_n_types_helper<P_, T_, 0>
  {
    using type = P_;
  };

public:
  using type = typename pack_n_types_helper<P<>, T, N>::type;
};

///
/// Get pack like type filled with types T as P<T_1, ..., T_N>.
/// \tparam P template pack type object to be filled with N types of type T
/// \tparam T type that shall be filled into P
/// \tparam N amount of types T in pack like pack P
///
template<template<typename...> typename P, typename T, std::size_t N>
using pack_n_types_t = typename pack_n_types<P, T, N>::type;

///
/// Deduce type in pack like type at index N.
///
template<pack_type P, std::size_t N>
struct type_from_pack;

template<template<typename...> typename P, std::size_t N, typename... Args>
struct type_from_pack<P<Args...>, N>
{
  using type = std::tuple_element_t<N, std::tuple<Args...>>;
};

///
/// Get the type of an element in pack like type P with index N.
/// \tparam P pack of types pack<...>
/// \tparam N index of type in pack as pack<T<0>, T<1>, ... T<N>, ... T<M>>
///
template<pack_type P, std::size_t N>
using type_from_pack_t = type_from_pack<P, N>::type;

///
/// Deduce index of specific type in pack like type.
///
template<pack_type P, typename T>
struct type_index;

template<template<typename...> typename P, typename... Args, typename T>
struct type_index<P<Args...>, T>
{
private:
  template<typename T_, typename Sequence_, typename... Args_>
  struct type_index_helper;

  template<typename T_, std::size_t I_, typename A_>
  struct type_index_helper<T_, std::index_sequence<I_>, A_>
  {
    using type = std::conditional_t<std::is_same_v<T_, A_>, detail::index_t<I_>, detail::index_t<I_ + 1>>;
  };

  template<typename T_, std::size_t I0_, std::size_t... I_, typename A_, typename... Args_>
  struct type_index_helper<T_, std::index_sequence<I0_, I_...>, A_, Args_...>
  {
    using type =
      std::conditional_t<std::is_same_v<T_, A_>, detail::index_t<I0_>, typename type_index_helper<T_, std::index_sequence<I_...>, Args_...>::type>;
  };

public:
  static constexpr std::size_t index = type_index_helper<T, std::make_index_sequence<sizeof...(Args)>, Args...>::type::index;
};

///
/// Get first type index of type T in pack P.
/// \tparam P pack type containing generic types
/// \tparam T type to be searched in pack P
/// \return index of T in pack, if P does not contain T, index is equal to the size of pack
///
template<pack_type P, typename T>
constexpr std::size_t type_index_v = type_index<P, T>::index;

///
/// Pack of arbitrary template parameters
/// \tparam ... arbitrary types
/// \result type of pack<...>
///
template<typename... Args>
struct pack
{};

///
/// Access general pack info via pack type.
///
template<>
struct pack<>
{
  static constexpr std::size_t size = 0;
  using first_type = void;
  using last_type = void;
};

template<typename T, typename... Args>
struct pack<T, Args...>
{
  static constexpr std::size_t size = sizeof...(Args) + 1;
  using first_type = T;
  using last_type = type_from_pack_t<std::tuple<T, Args...>, size - 1>;
};

///
/// Get size of a generic pack like type.
///
template<pack_type P>
struct pack_size;

template<template<typename...> typename P, typename... Args>
struct pack_size<P<Args...>>
{
  static constexpr auto value = sizeof...(Args);
};

///
/// Get sizeof...(Args) of generic pack P<Args...>.
/// \tparam P pack type containing generic types
/// \return sizeof...(Args) in pack P<Args...>
///
template<pack_type P>
constexpr std::size_t pack_size_v = pack_size<P>::value;

} // namespace bibstd::meta
