#include "src/app_velopack_startup.hpp"
#include "src/app_updater.hpp"

#include <Velopack.hpp>

#include <cstdlib>

namespace aba
{

///
///
app_velopack_startup::app_velopack_startup()
{
  // Exits the hooks, otherwise these would start the whole app.
  // Velopack's own exit runs the static destructors after the Qt dlls are unloaded, which crashes.
  const auto exit_hook = [](void* /*user_data*/, const char* /*version*/) { std::_Exit(EXIT_SUCCESS); };
  const auto restarted_hook = [](void* user_data, const char* /*version*/) { *static_cast<bool*>(user_data) = true; };
  Velopack::VelopackApp::Build()
    .SetAutoApplyOnStartup(false)
    .OnAfterInstall(exit_hook)
    .OnBeforeUninstall(exit_hook)
    .OnBeforeUpdate(exit_hook)
    .OnAfterUpdate(exit_hook)
    .OnRestarted(restarted_hook)
    .Run(&restarted_after_install_);
}

///
///
auto app_velopack_startup::install_pending_update() const -> bool
{
  return !restarted_after_install_ && install_downloaded_update();
}

} // namespace aba
