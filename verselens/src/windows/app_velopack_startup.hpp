#pragma once

namespace verselens
{

///
/// Velopack startup: runs the install, update and uninstall hooks and installs an update downloaded by an earlier run.
///
class app_velopack_startup final
{
  // Variables
  bool restarted_after_install_{false};

public: // Structors
  ///
  /// Run the Velopack hooks, they end the process in here. It has to be constructed first in main.
  ///
  app_velopack_startup();

public: // Modifiers
  ///
  /// Install an update downloaded by an earlier run, see install_downloaded_update().
  /// Skipped if Velopack just started the application after an install, so an install that failed does not restart
  /// the application over and over. Velopack's own install on startup skips it the same way, but it is off: it runs
  /// before the single instance check and would replace the files of a running instance.
  /// \note Call it once this process is the only instance.
  /// \return true if the update was handed over, the application has to exit then
  ///
  [[nodiscard]] auto install_pending_update() const -> bool;
};

} // namespace verselens
