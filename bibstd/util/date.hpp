#pragma once

#include <chrono>
#include <format>
#include <string>

namespace bibstd::util
{

///
/// Format current time in {:%F_%H-%M-%S} format.
/// \return CET time as string
///
inline auto format_current_time_CET() -> std::string
{
  const auto time_point = std::chrono::system_clock::now();
  static const auto CET = std::chrono::locate_zone("Etc/GMT-1");
  return std::format("{:%F_%H-%M-%S}", std::chrono::zoned_time{CET, std::chrono::floor<std::chrono::milliseconds>(time_point)});
}

} // namespace bibstd::util
