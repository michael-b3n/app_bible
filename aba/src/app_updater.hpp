#pragma once

#include <QProcess>
#include <QTimer>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>

// Forward declarations
namespace Velopack
{
class UpdateManager;
} // namespace Velopack

namespace aba
{

///
/// Updater: checks the GitHub releases for a new version while the application runs and downloads it.
/// The check and the download run in a process of their own, see run_update_download(). Destroying the updater ends
/// that process, a download it did not finish is discarded.
///
class updater final
{
  // Typedefs
  using downloaded_callback_type = std::function<void()>;

  // Constants
  // Starting at sign-in usually happens before the network is up
  static constexpr auto first_check_delay = std::chrono::minutes{3};
  static constexpr auto check_interval = std::chrono::hours{24};
  static constexpr auto retry_interval = std::chrono::hours{6};

  // Variables
  const downloaded_callback_type on_downloaded_;
  std::unique_ptr<Velopack::UpdateManager> manager_;
  std::optional<std::string> reported_version_;
  bool install_started_{false};
  QTimer timer_;
  QProcess download_;

public: // Structors
  ///
  /// Start checking for updates, unless this executable was not installed by Velopack.
  /// \p on_downloaded is called once per downloaded version.
  ///
  explicit updater(downloaded_callback_type on_downloaded);
  ~updater() noexcept;
  updater(const updater&) = delete;
  updater(updater&&) = delete;
  auto operator=(const updater&) -> updater& = delete;
  auto operator=(updater&&) -> updater& = delete;

public: // Modifiers
  ///
  /// Install the downloaded update, see install_downloaded_update().
  /// \return true if the update was handed over, which happens once
  ///
  [[nodiscard]] auto install_and_restart() -> bool;

private: // Implementation
  auto start_download() -> void;
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
