///
/// Main file.
///
#include "res/version.hpp"
#include "src/app_already_running.hpp"
#include "src/app_updater.hpp"
#include "src/app_velopack_startup.hpp"
#include "src/construct_backend.hpp"
#include "src/construct_bridge.hpp"
#include "src/construct_translations.hpp"
#include "src/construct_tray.hpp"
#include "src/construct_updater.hpp"
#include "src/qml_application.hpp"

#include <bibstd/framework/single_instance.hpp>
#include <bibstd/system/app_package.hpp>
#include <bibstd/system/filesystem.hpp>
#include <bibstd/system/screen.hpp>
#include <bibstd/util/log.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#include <cstdlib>
#include <format>
#include <memory>
#include <optional>

Q_IMPORT_QML_PLUGIN(BibQmlPlugin)

///
/// Main function.
///
int main(int argc, char** argv)
{
  // A packaged install is updated by the store it came from, Velopack must not touch its files.
  const auto packaged = bibstd::system::app_package::packaged();

  // Velopack hooks end the process in here, the updater's download process right after
  const auto velopack =
    packaged ? std::optional<verselens::app_velopack_startup>{} : std::optional<verselens::app_velopack_startup>{std::in_place};
  if(!packaged)
  {
    if(const auto exit_code = verselens::run_update_download(argc, argv); exit_code.has_value())
    {
      return *exit_code;
    }
  }

  // The instance check runs before the logger, a second instance would truncate the log of the running one.
  const auto instance = bibstd::framework::single_instance::claim(std::string{verselens::version::data_folder_name});
  if(!instance.is_owner())
  {
    return verselens::show_already_running(argc, argv);
  }

  const auto logger = bibstd::util::logger(
    verselens::version::data_folder_name,
    std::format(
      "executable: {}\nversion: {}\ncommit_hash: {}\ncommit_date: {}\npackaged: {}\n",
      bibstd::system::filesystem::executable_location().string(),
      verselens::version::version_string,
      verselens::version::commit_hash,
      verselens::version::commit_date,
      packaged
    )
  );
  if(const auto& single_instance_error = instance.error(); single_instance_error.has_value())
  {
    LOG_WARN("single instance guard inactive: {}", *single_instance_error);
  }

  // An update downloaded by an earlier run restarts the application into the new version
  if(velopack.has_value() && velopack->install_pending_update())
  {
    return EXIT_SUCCESS;
  }

  if(!bibstd::system::screen::init())
  {
    LOG_ERROR("failed to initialize screen settings");
    return EXIT_FAILURE;
  }

  // Init backend components.
  auto backend = verselens::construct_backend();

  // Initialize Qt application.
  verselens::configure_qml_layer();
  QGuiApplication app(argc, argv);

  // Note bridge and translations must be declared before engine so they outlive QML objects
  auto bridge = verselens::construct_bridge(app, backend);
  verselens::connect_bridge(bridge);

  // Init the pretty names of the frontend. The backend deals with identifiers only.
  auto translations = verselens::construct_translations(backend);

  // Keeps the installed app up to date, a packaged install gets its updates from the store.
  const auto updater =
    packaged ? std::unique_ptr<verselens::app_updater>{} : verselens::construct_updater(app, bridge, translations);

  QQmlApplicationEngine engine;

  verselens::connect_engine(engine, app, bridge);

  // Start system tray.
  const auto tray_guard = verselens::construct_tray(app, bridge, translations);

  const auto reval = QGuiApplication::exec();
  LOG_INFO("exit application: {}", reval);
  return reval;
}
