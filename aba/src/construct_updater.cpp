#include "src/construct_updater.hpp"
#include "src/qml_application.hpp"

#include <bibqml/bridge/BridgeApplication.hpp>

#include <QObject>

namespace aba
{

///
///
auto construct_updater(QGuiApplication& app, bridge_instance& bridge, translations_instance& translations)
  -> std::unique_ptr<app_updater>
{
  auto instance = std::make_unique<app_updater>(*bridge.bridge_application);
  QObject::connect(
    bridge.bridge_application.get(),
    &bibqml::BridgeApplication::updateRequested,
    &app,
    [&app, &bridge, &translations, updater = instance.get()]()
    {
      if(updater->install_and_restart())
      {
        quit_application(app, bridge, translations);
      }
    }
  );
  return instance;
}

} // namespace aba
