#pragma once

namespace bibstd::system
{

///
/// App package class for generic OS.
///
struct app_package final
{
  ///
  /// Check if the running executable is part of an installed app package, e.g. an MSIX package from the store.
  /// \return true if the executable runs from a package, false otherwise
  ///
  [[nodiscard]] static auto packaged() -> bool;
};

} // namespace bibstd::system
