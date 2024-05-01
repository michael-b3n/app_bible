#pragma once

#include <string>

namespace bibstd::util
{

///
/// Convert u8 string view to normal string.
/// \warning Normal strings are encoded in utf-8.
/// \param u8string that shall be converted
/// \return string with content of string
///
inline auto to_string(const std::u8string& u8string) -> std::string
{
  return std::string{u8string.begin(), u8string.end()};
}

} // namespace bibstd::util
