///
/// Main file.
///
#include "version.hpp"

#include <bibstd/framework/thread_pool.hpp>
#include <bibstd/system/filesystem.hpp>
#include <bibstd/system/hotkey.hpp>
#include <bibstd/system/open_browser.hpp>
#include <bibstd/system/screen.hpp>
#include <bibstd/system/tray.hpp>
#include <bibstd/util/date.hpp>
#include <bibstd/util/incbin.hpp>
#include <bibstd/util/log.hpp>

#include <bibstd/presenter/presenter_bible_ref_ocr.hpp>

#include <bibqml/BridgeBibleRefOcr.hpp>

#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <QtQml/QQmlExtensionPlugin>
Q_IMPORT_QML_PLUGIN(BibQmlPlugin)

#include <filesystem>
#include <format>

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

  if(bibstd::system::screen::init())
  {
    LOG_ERROR("failed to initialize screen settings");
    return EXIT_FAILURE;
  }

  // Init backend
  auto presenter_bible_ref_ocr = bibstd::presenter::presenter_bible_ref_ocr();

  // Init thread pool
  const auto pool_guard = bibstd::framework::thread_pool::init();

  // Init hotkey system
  const auto hotkey_guard = bibstd::system::hotkey::init();
  // Register hotkeys. Currently no hotkey change is supported.
  bibstd::system::hotkey::register_callback(
    presenter_bible_ref_ocr.settings->hotkey->value(),
    presenter_bible_ref_ocr.settings->hotkey_modifier->value(),
    [&presenter_bible_ref_ocr, stop_source = std::stop_source()]() mutable
    {
      stop_source.request_stop();
      const auto cursor_pos = bibstd::system::screen::cursor_position();
      stop_source = presenter_bible_ref_ocr.start(cursor_pos);
    }
  );

  // Initialize Qt application.
  QGuiApplication app(argc, argv);
  QQmlApplicationEngine engine;

  // Create OCR bridge and pass ownership to QML engine
  auto bridge_bible_ref_ocr = bibqml::BridgeBibleRefOcr(presenter_bible_ref_ocr);

  // Set initial properties for the QML root component
  engine.setInitialProperties({
    {"bridge", QVariant::fromValue(&bridge_bible_ref_ocr)}
  });

  QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    [](const QUrl& url)
    {
      LOG_INFO("qml object creation failed: url: \"{}\"", url.toString().toStdString());
      QCoreApplication::exit(EXIT_FAILURE);
    },
    Qt::QueuedConnection
  );
  engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ui/qml/main.qml")));

  // Connect tray signals
  const auto do_on_exit = [&]() { QMetaObject::invokeMethod(&app, [&app]{ app.quit(); }, Qt::QueuedConnection); };
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
