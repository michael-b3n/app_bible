///
/// Main file.
///
#include "res/version.hpp"
#include "src/construct_backend.hpp"
#include "src/construct_bridge.hpp"
#include "src/construct_translations.hpp"

#include <bibqml/bridge/BridgeApplication.hpp>

#include <bibstd/system/filesystem.hpp>
#include <bibstd/system/open_browser.hpp>
#include <bibstd/system/screen.hpp>
#include <bibstd/system/tray.hpp>
#include <bibstd/util/incbin.hpp>
#include <bibstd/util/log.hpp>

#include <QGuiApplication>
#include <QMetaObject>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QtQml/QQmlExtensionPlugin>

Q_IMPORT_QML_PLUGIN(BibQmlPlugin)

INC_RESOURCE(icon, "res/icon.ico");
const auto icon_view = bibstd::util::incbin::to_span<std::byte>(res_icon_data, res_icon_size);

///
/// Main function.
///
int main(int argc, char** argv)
{
  const auto logger = bibstd::util::logger();
  LOG_INFO("executable: {}", bibstd::system::filesystem::executable_location().string());
  LOG_INFO("version: {}", aba::version::version_string);
  LOG_INFO("commit_hash: {}", aba::version::commit_hash);
  LOG_INFO("commit_date: {}", aba::version::commit_date);

  if(!bibstd::system::screen::init())
  {
    LOG_ERROR("failed to initialize screen settings");
    return EXIT_FAILURE;
  }

  // Init backend components.
  auto backend = aba::construct_backend();

  // Initialize Qt application.
#ifdef _WIN32
  QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
#endif
  QQuickWindow::setTextRenderType(QQuickWindow::CurveTextRendering);
  QGuiApplication app(argc, argv);
  // The controls of this application are fully styled by the qml layer. Without an explicit
  // style, the platform style is used, which draws its own hover and scroll visuals on top of
  // the custom ones. The basic style is the neutral style that leaves the controls untouched.
  QQuickStyle::setStyle("Basic");
  LOG_INFO("qml controls style: \"{}\"", QQuickStyle::name().toStdString());

  // Note bridge and translations must be declared before engine so they outlive QML objects
  auto bridge = aba::construct_bridge(app, backend);
  aba::connect_bridge(bridge);

  // Init the pretty names of the frontend. The backend deals with identifiers only.
  auto translations = aba::construct_translations(backend);

  QQmlApplicationEngine engine;

  aba::connect_engine(engine, app, bridge);

  // Connect tray signals
  const auto do_on_exit = [&]()
  {
    aba::disconnect_bridge(bridge);
    translations.disconnect();
    QMetaObject::invokeMethod(&app, [&app] { app.quit(); }, Qt::QueuedConnection);
  };
  const auto open_github = []() { bibstd::system::open_browser::open("https://github.com/michael-b3n/app_bible"); };
  const auto show_window = [&bridge]() { bridge.bridge_application->requestShowWindow(); };
  // Start system tray.
  const auto tray_guard = bibstd::system::tray::init(
    bibstd::system::tray::icon_buffer{icon_view},
    {
      bibstd::system::tray::entry_type{bibstd::system::tray::button{"Show window", show_window}},
      bibstd::system::tray::entry_type{bibstd::system::tray::button{"Open GitHub", open_github}},
      bibstd::system::tray::entry_type{bibstd::system::tray::button{"Exit", do_on_exit}},
      // ...
    }
  );

  const auto reval = app.exec();
  LOG_INFO("exit application: {}", reval);
  return reval;
}
