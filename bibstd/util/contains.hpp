#pragma once

#include <algorithm>
#include <concepts>

namespace bibstd::util
{

///
/// Check if an element is contained in a container.
/// \tparam Container type
/// \tparam Container value type
/// \param container Container that shall be checked
/// \param element Element of container that might be contained in the container
/// \return true if `element` is found in container, false if not
///
template<typename Container>
constexpr auto contains(const Container& container, const std::ranges::range_value_t<Container>& element) -> bool
{
  return std::ranges::find(container, element) != std::ranges::cend(container);
}

///
/// Check if an element is inside a container or not.
/// \tparam View type
/// \tparam F predicate
/// \param view View on container
/// \param predicate function
/// \return bool true if element is found in container, false otherwise
///
template<std::ranges::view View, typename F>
  requires std::predicate<F, std::ranges::range_value_t<View>>
constexpr auto contains(View view, F&& pred) -> bool
{
  return std::ranges::find_if(view, std::forward<F>(pred)) != std::ranges::cend(view);
}

} // namespace bibstd::util
