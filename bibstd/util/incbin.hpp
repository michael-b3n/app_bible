#pragma once

#define INCBIN_STYLE  INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX res_
extern "C"
{
#include "incbin.h"
}
#include <cstddef>
#include <span>
#include <string_view>
#include <type_traits>

///
/// Usage: INC_RESOURCE(LABEL, FILE)
///
/// Symbols defined by INCBIN:
/// `const std::byte res_{LABEL}_data[];`
/// `const std::byte* const res_{LABEL}_end;`
/// `const unsigned int res_{LABEL}_size;`
///
#define INC_RESOURCE(LABEL, FILE) INCBIN(std::byte, LABEL, FILE)

namespace bibstd::util::incbin
{

///
/// Reinterpret data pointer and size with string view.
/// \param data Byte data pointer
/// \param size Size of pointer array
/// \return `std::string_view` on data
///
inline auto to_string_view(const std::byte* const data, const unsigned int size) -> std::string_view
{
  return std::string_view(reinterpret_cast<const char* const>(data), size);
}

///
/// Reinterpret data pointer and size with span of given type `T`.
/// \tparam T value_type of span
/// \param data Byte data pointer
/// \param size Size of pointer array
/// \return `std::span<T>` on data
///
template<typename T>
inline auto to_span(const std::byte* const data, const unsigned int size) -> std::span<const T>
{
  return std::span(reinterpret_cast<const T* const>(data), size / sizeof(T));
}

} // namespace bibstd::util::incbin
