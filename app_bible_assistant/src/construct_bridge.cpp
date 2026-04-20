#include "src/construct_bridge.hpp"

#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

#include <bibqml/AbstractListModelPassage.hpp>
#include <bibqml/BridgeBibleRefOcr.hpp>
#include <memory>

namespace aba
{

///
///
auto disconnect_bridge(bridge_instance& instance) -> void
{
  assert(instance.bridge_bible_ref_ocr);
  instance.bridge_bible_ref_ocr->disconnect();
}

///
///
auto construct_bridge(QGuiApplication& app, QQmlApplicationEngine& engine, backend_instance& backend) -> bridge_instance
{
  // clang-format off
  auto workflow_bible_ref_ocr = std::static_pointer_cast<bibstd::workflow::workflow_bible_ref_ocr>(backend.workflow_bible_ref_ocr);
  auto workflow_hotkey = std::static_pointer_cast<bibstd::workflow::workflow_hotkey>(backend.workflow_hotkey);
  auto workflow_scripture = std::static_pointer_cast<bibstd::workflow::workflow_scripture>(backend.workflow_scripture);
  // clang-format on
  auto bibqml_inst = bridge_instance{
    .bridge_bible_ref_ocr{std::make_unique<bibqml::BridgeBibleRefOcr>(workflow_bible_ref_ocr, workflow_hotkey)},
    .abstract_list_model_passage{std::make_unique<bibqml::AbstractListModelPassage>(workflow_scripture)}
  };

  // Connect bridge signal to passage model
  QObject::connect(
    bibqml_inst.bridge_bible_ref_ocr.get(),
    &bibqml::BridgeBibleRefOcr::referenceFound,
    bibqml_inst.abstract_list_model_passage.get(),
    &bibqml::AbstractListModelPassage::resetWithReference
  );

  // Set initial properties for the QML root component
  engine.setInitialProperties({
    {"bridgeBibleRefOcr",        QVariant::fromValue(bibqml_inst.bridge_bible_ref_ocr.get())},
    { "listModelPassage", QVariant::fromValue(bibqml_inst.abstract_list_model_passage.get())}
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

} // namespace aba
