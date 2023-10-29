#pragma once

#include "system/filesystem_base.hpp"

#include <filesystem>
#include <optional>

namespace bibstd::system
{

///
/// Filesystem class for windows implementation.
///
struct filesystem final : public filesystem_base
{
  ///
  /// Get path to local data folder.
  /// \return path to local data
  ///
  static inline auto local_data_folder() -> std::filesystem::path;
};

///
///
inline auto filesystem::local_data_folder() -> std::filesystem::path
{
  auto appdata = std::getenv("LOCALAPPDATA");
  if(!appdata)
  {
    throw std::runtime_error("local appdata not found");
  }
  return std::filesystem::path(appdata) / executable_location().stem();
}

} // namespace bibstd::system
