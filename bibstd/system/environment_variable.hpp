#pragma once

#include <string>

namespace bibstd::system
{

///
/// Environment variable class for generic OS.
///
struct environment_variable final
{
  ///
  /// Set an environment variable.
  /// \param name Name of the environment variable
  /// \param value Value of the environment variable
  /// \return true if the operation was successful, false otherwise
  ///
  static auto set(const std::string& name, const std::string& value) -> bool;
};

} // namespace bibstd::system
