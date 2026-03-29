///
/// Main file.
///
#include "res/version.hpp"
#include "src/construct_backend.hpp"
#include "src/construct_bridge.hpp"

#include <bibstd/framework/thread_pool.hpp>
#include <bibstd/system/filesystem.hpp>
#include <bibstd/system/hotkey.hpp>
#include <bibstd/system/open_browser.hpp>
#include <bibstd/system/screen.hpp>
#include <bibstd/system/tray.hpp>
#include <bibstd/util/date.hpp>
#include <bibstd/util/incbin.hpp>
#include <bibstd/util/log.hpp>

#include <bibqml/BridgeBibleRefOcr.hpp>

#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>

#include <QtQml/QQmlExtensionPlugin>
Q_IMPORT_QML_PLUGIN(BibQmlPlugin)

#include <filesystem>

INC_RESOURCE(icon, "res/icon.ico");
const auto icon_view = bibstd::util::incbin::to_span<std::byte>(res_icon_data, res_icon_size);

///
/// Main function.
///
int main(int argc, char** argv)
{
  const auto logger = bibstd::util::logger();
  LOG_INFO("executable: {}", bibstd::system::filesystem::executable_location().string());
  LOG_INFO("version: {}", bible_assistant::version::version_string);
  LOG_INFO("commit_hash: {}", bible_assistant::version::commit_hash);
  LOG_INFO("commit_date: {}", bible_assistant::version::commit_date);

  if(!bibstd::system::screen::init())
  {
    LOG_ERROR("failed to initialize screen settings");
    return EXIT_FAILURE;
  }

  // Init backend components.
  auto backend = bible_assistant::construct_backend();

  // Initialize Qt application.
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  // Create OCR bridge and pass ownership to QML engine
  auto bridge = bible_assistant::construct_bridge(app, engine, backend);

  // Connect tray signals
  const auto do_on_exit = [&]() { QMetaObject::invokeMethod(&app, [&app] { app.quit(); }, Qt::QueuedConnection); };
  const auto open_github = []() { bibstd::system::open_browser::open("https://github.com/michael-b3n/app_bible"); };
  // Start system tray.
  const auto tray_guard = bibstd::system::tray::init(
    bibstd::system::tray::icon_buffer{icon_view},
    {
      bibstd::system::tray::entry_type{bibstd::system::tray::button{"Exit", do_on_exit}},
      bibstd::system::tray::entry_type{bibstd::system::tray::button{"Open GitHub", open_github}},
      // ...
    }
  );

  const auto reval = app.exec();
  LOG_INFO("exit application: {}", reval);
  return reval;
}
