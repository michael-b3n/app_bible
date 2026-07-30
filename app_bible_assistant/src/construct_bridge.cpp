#include "src/construct_bridge.hpp"

#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <bibqml/bridge/BridgeBibleRefOcr.hpp>
#include <bibqml/bridge/BridgeSettings.hpp>
#include <bibqml/model/ScriptureListModel.hpp>
#include <bibqml/model/SettingsListModel.hpp>

#include <memory>

namespace aba
{

///
///
bridge_instance::~bridge_instance() noexcept = default;

///
///
auto disconnect_bridge(bridge_instance& instance) -> void
{
  assert(instance.bridge_bible_ref_ocr);
  instance.bridge_bible_ref_ocr->disconnect();
  instance.settings_list_model->disconnect();
}

///
///
auto construct_bridge(QGuiApplication& app, backend_instance& backend) -> bridge_instance
{
  // clang-format off
  auto workflow_settings = backend.workflow_settings;
  auto workflow_bible_ref_ocr = std::static_pointer_cast<bibstd::workflow::workflow_bible_ref_ocr>(backend.workflow_bible_ref_ocr);
  auto workflow_hotkey = std::static_pointer_cast<bibstd::workflow::workflow_hotkey>(backend.workflow_hotkey);
  auto workflow_scripture = std::static_pointer_cast<bibstd::workflow::workflow_scripture>(backend.workflow_scripture);
  // clang-format on
  return bridge_instance{
    .bridge_settings{std::make_unique<bibqml::BridgeSettings>(workflow_settings)},
    .settings_list_model{std::make_unique<bibqml::SettingsListModel>(workflow_settings)},
    .bridge_bible_ref_ocr{std::make_unique<bibqml::BridgeBibleRefOcr>(workflow_bible_ref_ocr, workflow_hotkey)},
    .scripture_list_model{std::make_unique<bibqml::ScriptureListModel>(workflow_scripture)}
  };
}

///
///
auto connect_bridge(bridge_instance& instance) -> void
{
  // Connect bridge signal to passage model
  QObject::connect(
    instance.bridge_bible_ref_ocr.get(),
    &bibqml::BridgeBibleRefOcr::referenceFound,
    instance.scripture_list_model.get(),
    &bibqml::ScriptureListModel::resetWithReference
  );

  // Clear passage model when OCR starts
  QObject::connect(
    instance.bridge_bible_ref_ocr.get(),
    &bibqml::BridgeBibleRefOcr::runningChanged,
    instance.scripture_list_model.get(),
    [model = instance.scripture_list_model.get()](bool running)
    {
      if(running)
      {
        model->clear();
      }
    }
  );
}

///
///
auto connect_engine(QQmlApplicationEngine& engine, QGuiApplication& app, bridge_instance& bridge) -> void
{
  // Set initial properties for the QML root component
  engine.setInitialProperties({
    { "listModelSettings",  QVariant::fromValue(bridge.settings_list_model.get())},
    {"listModelScripture", QVariant::fromValue(bridge.scripture_list_model.get())},
    { "bridgeBibleRefOcr", QVariant::fromValue(bridge.bridge_bible_ref_ocr.get())},
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
  engine.load(QUrl(QStringLiteral("qrc:/qt/qml/ui/qml/Main.qml")));
}

} // namespace aba
