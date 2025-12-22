#pragma once

#include <boost/dll.hpp>
#include <filesystem>

namespace bibstd::system
{

///
/// Filesystem class for windows implementation.
///
struct filesystem final
{
  ///
  /// Get path to executable.
  /// \return path to executable location
  ///
  static inline auto executable_location() -> std::filesystem::path;

  ///
  /// Get path to executable folder.
  /// \return path to executable folder
  ///
  static inline auto executable_folder() -> std::filesystem::path;

  ///
  /// Get path to local data folder.
  /// \return path to local data
  ///
  static auto local_data_folder() -> std::filesystem::path;
};

///
///
inline auto filesystem::executable_location() -> std::filesystem::path
{
  return boost::dll::program_location().string();
}

///
///
inline auto filesystem::executable_folder() -> std::filesystem::path
{
  return boost::dll::program_location().parent_path().string();
}

} // namespace bibstd::system
