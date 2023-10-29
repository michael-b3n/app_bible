#pragma once

#include <type_traits>

namespace bibstd::meta
{

///
/// Convert type T to R if T is same type as S.
///
template<typename T, typename S, typename R>
struct convert_if
{
  using type = std::conditional_t<std::is_same_v<T, S>, R, T>;
};
template<typename T, typename S, typename R>
using convert_if_t = typename convert_if<T, S, R>::type;

} // namespace bibstd::meta
