#include "src/construct_updater.hpp"
#include "src/qml_application.hpp"

#include <bibqml/bridge/BridgeApplication.hpp>

#include <QObject>

namespace aba
{

///
///
auto construct_updater(QGuiApplication& app, bridge_instance& bridge, translations_instance& translations)
  -> std::unique_ptr<updater>
{
  auto* const bridge_application = bridge.bridge_application.get();
  auto instance = std::make_unique<updater>([bridge_application]() { bridge_application->notifyUpdateAvailable(); });

  QObject::connect(
    bridge_application,
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
