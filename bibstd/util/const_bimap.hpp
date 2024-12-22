#pragma once

#include "meta/pack.hpp"
#include "meta/type_traits.hpp"
#include "util/exception.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <type_traits>
#include <utility>

namespace bibstd::util
{
namespace detail
{

template<typename T>
struct is_pair : std::false_type
{};
template<typename T1, typename T2>
struct is_pair<std::pair<T1, T2>> : std::true_type
{};
template<typename T>
constexpr bool is_pair_v = is_pair<T>::value;

template<typename T>
struct is_array : std::false_type
{};
template<typename T, std::size_t N>
struct is_array<std::array<T, N>> : std::true_type
{};
template<typename T>
constexpr bool is_array_v = is_array<T>::value;

template<typename T>
concept is_valid_bimap_type = requires {
  requires is_array_v<T>;
  requires is_pair_v<typename T::value_type>;
  requires !std::is_same_v<typename T::value_type::first_type, typename T::value_type::second_type>;
  requires std::equality_comparable<typename T::value_type::first_type>;
  requires std::equality_comparable<typename T::value_type::second_type>;
};

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
  using size_type = T::size_type;
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
  /// Checks if first value is contained in const bimap.
  /// \tparam F type must be equality comparable with first_type or first_type if first and second types are similar.
  /// \param first value
  /// \return true, if first value was found, false otherwise
  ///
  template<typename F>
  [[nodiscard]] constexpr auto contains(const F& first) const -> bool
    requires detail::explicit_if_similar<F, first_type, second_type> && std::equality_comparable_with<F, first_type>;

  ///
  /// Checks if second value is contained in const bimap.
  /// \tparam F type must be equality comparable with second_type or second_type if first and second types are similar.
  /// \param second value
  /// \return true, if second value was found, false otherwise
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

  ///
  /// Tells how many entries are in the bimap.
  /// \return size of bimap
  ///
  [[nodiscard]] constexpr auto size() const -> size_type;

public: // Iterator Overloads
  constexpr auto begin() const -> const_iterator { return map_.begin(); }
  constexpr auto cbegin() const -> const_iterator { return map_.cbegin(); }
  constexpr auto end() const -> const_iterator { return map_.end(); }
  constexpr auto cend() const -> const_iterator { return map_.cend(); }
  constexpr auto rbegin() const -> const_reverse_iterator { return map_.rbegin(); }
  constexpr auto crbegin() const -> const_reverse_iterator { return map_.crbegin(); }
  constexpr auto rend() const -> const_reverse_iterator { return map_.rend(); }
  constexpr auto crend() const -> const_reverse_iterator { return map_.crend(); }

private: // Typedefs
  ///
  /// Compile time pointer comparisons are not defined, even though with GCC a constexpr const_bimap with pairs
  /// of type pair<C, const char*> will compile. Other compilers like clang will not.
  /// The comparable_type templated alias should be used for comparing elements.
  ///
  template<typename C>
  using comparable_type = std::conditional_t<std::is_convertible_v<C, std::string_view>, std::string_view, C>;

private: // Implementation
  ///
  /// Checks if values are equal using `comparable_type` for comparison.
  /// \param lhs Left side
  /// \param rhs Right side
  /// \return true if equal, false otherwise
  ///
  constexpr auto is_equal(const auto& lhs, const auto& rhs) const -> bool;

private: // Variables
  const T map_;
};

///
/// Const bimap type check.
///
template<typename T>
struct is_const_bimap : std::false_type
{};
template<typename T>
struct is_const_bimap<const_bimap<T>> : std::true_type
{};
template<typename T>
constexpr bool is_const_bimap_v = is_const_bimap<T>::value;

///
/// Const bimap type concept.
///
template<typename T>
concept const_bimap_type = is_const_bimap_v<T>;

///
///
template<typename... P>
  requires meta::are_same_v<P...> && detail::is_pair_v<typename meta::pack<P...>::first_type>
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
    std::views::iota(std::size_t{0}, sizeof...(P)),
    [&](const auto i)
    {
      std::ranges::for_each(
        std::views::iota(i + 1, sizeof...(P)),
        [&](const auto j)
        {
          if(is_equal(map_.at(i).first, map_.at(j).first))
          {
            THROW_EXCEPTION(util::exception("Duplicates in first elements."));
          }
          if(is_equal(map_.at(i).second, map_.at(j).second))
          {
            THROW_EXCEPTION(util::exception("Duplicates in second elements."));
          }
        });
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
  return std::ranges::find_if(map_, [&](const auto& e) { return is_equal(first, e.first); }) != std::cend(map_);
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
           template<typename S>
           constexpr auto const_bimap<T>::contains(const S& second) const -> bool
             requires detail::explicit_if_similar<S, second_type, first_type> && std::equality_comparable_with<S, second_type>
{
  return std::ranges::find_if(map_, [&](const auto& e) { return is_equal(second, e.second); }) != std::cend(map_);
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
           template<typename F>
           constexpr auto const_bimap<T>::at(const F& first) const -> const second_type&
             requires detail::explicit_if_similar<F, first_type, second_type> && std::equality_comparable_with<F, first_type>
{
  const auto iter = std::ranges::find_if(map_, [&](const auto& e) { return is_equal(first, e.first); });
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
  const auto iter = std::ranges::find_if(map_, [&](const auto& e) { return is_equal(second, e.second); });
  if(iter == std::cend(map_))
  {
    THROW_EXCEPTION(util::exception("Second out of range."));
  }
  return iter->first;
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
constexpr auto const_bimap<T>::size() const -> size_type
{
  return map_.size();
}

///
///
template<typename T>
  requires detail::is_valid_bimap_type<T>
constexpr auto const_bimap<T>::is_equal(const auto& lhs, const auto& rhs) const -> bool
{
  return static_cast<comparable_type<decltype(lhs)>>(lhs) == static_cast<comparable_type<decltype(rhs)>>(rhs);
}

} // namespace bibstd::util
