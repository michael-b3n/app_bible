#pragma once

#include "meta/pack.hpp"
#include "meta/type_traits.hpp"
#include "util/exception.hpp"

#include <algorithm>
#include <array>
#include <type_traits>
#include <utility>

namespace bibstd::util
{
namespace detail
{

template<typename T>
concept is_valid_bimap_type = meta::is_array_v<T> && meta::is_pair_v<typename T::value_type> &&
                              (!std::is_same_v<typename T::value_type::first_type, typename T::value_type::second_type>);

template<typename T, typename F, typename S>
concept explicit_if_similar = std::is_same_v<T, std::conditional_t<std::equality_comparable_with<F, S>, F, T>>;

} // namespace detail

///
/// Const bimap with compile time access to values.
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
class const_bimap final
{
public: // Typedefs
  using value_type = T::value_type;
  using first_type = value_type::first_type;
  using second_type = value_type::second_type;
  using const_iterator = typename T::const_iterator;
  using const_reverse_iterator = typename T::const_reverse_iterator;

public: // Constructor
  ///
  /// Constructor of const bimap.
  /// \tparam ...P pack of type pair
  /// \param ...p pairs
  ///
  template<typename... P>
  constexpr const_bimap(P&&... p)
    requires meta::are_same_v<std::pair<first_type, second_type>, P...>;

public: // Accessor
  ///
  /// Checks if const bi map contains element of first type.
  /// \tparam F type must be equality comparable with first_type or first_type if first and second types are similar.
  /// \param first value
  /// \return true of first type element is contained, false otherwise
  ///
  template<typename F>
  [[nodiscard]] constexpr auto contains(const F& first) const -> bool
    requires detail::explicit_if_similar<F, first_type, second_type> && std::equality_comparable_with<F, first_type>;

  ///
  /// Checks if const bi map contains element of second type.
  /// \tparam F type must be equality comparable with second_type or second_type if first and second types are similar.
  /// \param second value
  /// \return true of second type element is contained, false otherwise
  ///
  template<typename S>
  [[nodiscard]] constexpr auto contains(const S& second) const -> bool
    requires detail::explicit_if_similar<S, second_type, first_type> && std::equality_comparable_with<S, second_type>;

  ///
  /// Access second value corresponding to first value.
  /// \tparam F type must be equality comparable with first_type or first_type if first and second types are similar.
  /// \param first value
  /// \return const reference to second element of pair corresponding to first
  ///
  template<typename F>
  [[nodiscard]] constexpr auto at(const F& first) const -> const second_type&
    requires detail::explicit_if_similar<F, first_type, second_type> && std::equality_comparable_with<F, first_type>;

  ///
  /// Access first value corresponding to second value.
  /// \tparam F type must be equality comparable with second_type or second_type if first and second types are similar.
  /// \param second value
  /// \return const reference to first element of pair corresponding to second
  ///
  template<typename S>
  [[nodiscard]] constexpr auto at(const S& second) const -> const first_type&
    requires detail::explicit_if_similar<S, second_type, first_type> && std::equality_comparable_with<S, second_type>;

public: // Iterator Overloads
  constexpr auto begin() const -> const_iterator { return map_.begin(); }
  constexpr auto cbegin() const -> const_iterator { return map_.cbegin(); }
  constexpr auto end() const -> const_iterator { return map_.end(); }
  constexpr auto cend() const -> const_iterator { return map_.cend(); }
  constexpr auto rbegin() const -> const_reverse_iterator { return map_.rbegin(); }
  constexpr auto crbegin() const -> const_reverse_iterator { return map_.crbegin(); }
  constexpr auto rend() const -> const_reverse_iterator { return map_.rend(); }
  constexpr auto crend() const -> const_reverse_iterator { return map_.crend(); }

private: // Variables
  const T map_;
};

template<typename... P>
  requires meta::are_same_v<P...> && meta::is_pair_v<typename meta::pack<P...>::first_type>
const_bimap(P&&... p) -> const_bimap<typename std::array<typename meta::pack<P...>::first_type, sizeof...(P)>>;

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
template<typename... P>
constexpr const_bimap<T>::const_bimap(P&&... p)
  requires meta::are_same_v<std::pair<first_type, second_type>, P...>
  : map_{std::array{std::forward<P>(p)...}}
{
  std::ranges::for_each(
    map_,
    [&](const auto& p)
    {
      if(std::ranges::count_if(map_, [&](const auto e) { return e.first == p.first; }) != 1)
      {
        THROW_EXCEPTION(util::exception("Duplicates in first elements."));
      }
      if(std::ranges::count_if(map_, [&](const auto e) { return e.second == p.second; }) != 1)
      {
        THROW_EXCEPTION(util::exception("Duplicates in second elements."));
      }
    });
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
           template<typename F>
           constexpr auto const_bimap<T>::contains(const F& first) const -> bool
             requires detail::explicit_if_similar<F, first_type, second_type> && std::equality_comparable_with<F, first_type>
{
  const auto iter = std::ranges::find_if(map_, [&](const auto& e) { return first == e.first; });
  return iter == std::cend(map_);
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
           template<typename S>
           constexpr auto const_bimap<T>::contains(const S& second) const -> bool
             requires detail::explicit_if_similar<S, second_type, first_type> && std::equality_comparable_with<S, second_type>
{
  const auto iter = std::ranges::find_if(map_, [&](const auto& e) { return second == e.second; });
  return iter == std::cend(map_);
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
           template<typename F>
           constexpr auto const_bimap<T>::at(const F& first) const -> const second_type&
             requires detail::explicit_if_similar<F, first_type, second_type> && std::equality_comparable_with<F, first_type>
{
  const auto iter = std::ranges::find_if(map_, [&](const auto& e) { return first == e.first; });
  if(iter == std::cend(map_))
  {
    THROW_EXCEPTION(util::exception("First out of range."));
  }
  return iter->second;
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
           template<typename S>
           constexpr auto const_bimap<T>::at(const S& second) const -> const first_type&
             requires detail::explicit_if_similar<S, second_type, first_type> && std::equality_comparable_with<S, second_type>
{
  const auto iter = std::ranges::find_if(map_, [&](const auto& e) { return second == e.second; });
  if(iter == std::cend(map_))
  {
    THROW_EXCEPTION(util::exception("Second out of range."));
  }
  return iter->first;
}

} // namespace bibstd::util
