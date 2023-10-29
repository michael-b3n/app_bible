#pragma once

#include <boost/dll.hpp>
#include <filesystem>
#include <string_view>

namespace bibstd::system
{

///
/// Filesystem base class for all system implementations.
///
struct filesystem_base
{
  virtual ~filesystem_base() noexcept = default;

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
};

///
///
inline auto filesystem_base::executable_location() -> std::filesystem::path
{
  return boost::dll::program_location().string();
}

///
///
inline auto filesystem_base::executable_folder() -> std::filesystem::path
{
  return boost::dll::program_location().parent_path().string();
}

} // namespace bibstd::system
