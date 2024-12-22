#pragma once

#include "meta/pack.hpp"

namespace bibstd::meta
{

///
/// Checks if pack like type P contains a type T
/// \tparam P pack like type
/// \tparam T generic type
/// \result true, if T is contained in P, false otherwise
///
template<pack_type P, typename T>
constexpr bool contains_v = type_index_v<P, T> < pack_size_v<P>;

} // namespace bibstd::meta
