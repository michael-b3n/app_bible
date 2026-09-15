#pragma once

#include <QProcess>
#include <QTimer>

#include <chrono>
#include <memory>
#include <optional>

// Forward declarations
namespace Velopack
{
class UpdateManager;
} // namespace Velopack

namespace bibqml
{
class BridgeApplication;
} // namespace bibqml

namespace aba
{

///
/// Updater: checks the GitHub releases for a new version while the application runs and downloads it.
/// The check and the download run in a process of their own, see run_update_download(). Destroying the updater ends
/// that process, a download it did not finish is discarded.
///
class app_updater final
{
  // Constants
  // Starting at sign-in usually happens before the network is up
  static constexpr auto first_check_delay = std::chrono::minutes{3};
  static constexpr auto check_interval = std::chrono::hours{24};
  static constexpr auto retry_interval = std::chrono::hours{6};
  // Velopack sets no network timeouts, a stalled connection would block all later checks
  static constexpr auto download_timeout = std::chrono::hours{2};

  // Variables
  bibqml::BridgeApplication& bridge_;
  std::unique_ptr<Velopack::UpdateManager> manager_;
  bool install_started_{false};
  // Times the next check while no download runs, the download timeout while one does
  QTimer timer_;
  QProcess download_;

public: // Structors
  ///
  /// Start checking for updates, unless this executable was not installed by Velopack.
  /// A downloaded update is reported to \p bridge, which has to outlive the updater.
  ///
  explicit app_updater(bibqml::BridgeApplication& bridge);
  ~app_updater() noexcept;
  app_updater(const app_updater&) = delete;
  app_updater(app_updater&&) = delete;
  auto operator=(const app_updater&) -> app_updater& = delete;
  auto operator=(app_updater&&) -> app_updater& = delete;

public: // Modifiers
  ///
  /// Install the downloaded update, see install_downloaded_update().
  /// \return true if the update was handed over, which happens once
  ///
  [[nodiscard]] auto install_and_restart() -> bool;

private: // Implementation
  auto timer_elapsed() -> void;
  auto download_finished(int exit_code, QProcess::ExitStatus exit_status) -> void;
};

///
/// Check for an update and download it, if the updater started this process to do so.
/// \return exit code of the process, std::nullopt if this process runs the application
///
[[nodiscard]] auto run_update_download(int argc, char** argv) -> std::optional<int>;

///
/// Hand the downloaded update to Velopack. It waits for this process to exit, installs the update and starts the
/// application again, so the caller has to quit the application next.
/// \return true if the update was handed over, false if there is none or this executable was not installed by Velopack
///
[[nodiscard]] auto install_downloaded_update() -> bool;

} // namespace aba
