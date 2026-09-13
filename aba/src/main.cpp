///
/// Main file.
///
#include "res/version.hpp"
#include "src/construct_backend.hpp"
#include "src/construct_bridge.hpp"
#include "src/construct_translations.hpp"
#include "src/construct_tray.hpp"
#include "src/qml_application.hpp"
#include "src/show_already_running.hpp"

#include <bibstd/framework/single_instance.hpp>
#include <bibstd/system/filesystem.hpp>
#include <bibstd/system/screen.hpp>
#include <bibstd/util/log.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#include <format>

Q_IMPORT_QML_PLUGIN(BibQmlPlugin)

///
/// Main function.
///
int main(int argc, char** argv)
{
  // The instance check runs before the logger, a second instance would truncate the log of the running one.
  const auto instance = bibstd::framework::single_instance::claim(std::string{aba::version::data_folder_name});
  if(!instance.is_owner())
  {
    return aba::show_already_running(argc, argv);
  }

  const auto logger = bibstd::util::logger(
    aba::version::data_folder_name,
    std::format(
      "executable: {}\nversion: {}\ncommit_hash: {}\ncommit_date: {}\n",
      bibstd::system::filesystem::executable_location().string(),
      aba::version::version_string,
      aba::version::commit_hash,
      aba::version::commit_date
    )
  );
  if(const auto& single_instance_error = instance.error(); single_instance_error.has_value())
  {
    LOG_WARN("single instance guard inactive: {}", *single_instance_error);
  }

  if(!bibstd::system::screen::init())
  {
    LOG_ERROR("failed to initialize screen settings");
    return EXIT_FAILURE;
  }

  // Init backend components.
  auto backend = aba::construct_backend();

  // Initialize Qt application.
  aba::configure_qml_layer();
  QGuiApplication app(argc, argv);

  // Note bridge and translations must be declared before engine so they outlive QML objects
  auto bridge = aba::construct_bridge(app, backend);
  aba::connect_bridge(bridge);

  // Init the pretty names of the frontend. The backend deals with identifiers only.
  auto translations = aba::construct_translations(backend);

  QQmlApplicationEngine engine;

  aba::connect_engine(engine, app, bridge);

  // Start system tray.
  const auto tray_guard = aba::construct_tray(app, bridge, translations);

  const auto reval = QGuiApplication::exec();
  LOG_INFO("exit application: {}", reval);
  return reval;
}
