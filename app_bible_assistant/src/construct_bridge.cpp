#include "src/construct_bridge.hpp"

#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>

#include <bibqml/BridgeBibleRefOcr.hpp>
#include <memory>

namespace bible_assistant
{

///
///
auto construct_bridge(QGuiApplication& app, QQmlApplicationEngine& engine, backend_instance& backend) -> bridge_instance
{
  // Create OCR bridge and pass ownership to QML engine

  // clang-format off
  auto workflow_bible_ref_ocr = std::static_pointer_cast<bibstd::workflow::workflow_bible_ref_ocr>(backend.workflow_bible_ref_ocr);
  auto workflow_hotkey = std::static_pointer_cast<bibstd::workflow::workflow_hotkey>(backend.workflow_hotkey);
  // clang-format on
  auto bibqml_inst = bridge_instance{
    .bridge_bible_ref_ocr = std::make_unique<bibqml::BridgeBibleRefOcr>(workflow_bible_ref_ocr, workflow_hotkey)
  };

  // Set initial properties for the QML root component
  engine.setInitialProperties({
    {"bridgeBibleRefOcr", QVariant::fromValue(bibqml_inst.bridge_bible_ref_ocr.get())}
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
  return bibqml_inst;
}

} // namespace bible_assistant
