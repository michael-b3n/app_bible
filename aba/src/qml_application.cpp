#include "src/qml_application.hpp"

#include <bibstd/util/log.hpp>

#include <QQuickStyle>
#include <QQuickWindow>

namespace aba
{

///
///
auto configure_qml_layer() -> void
{
#ifdef _WIN32
  QQuickWindow::setGraphicsApi(QSGRendererInterface::Direct3D11);
#endif
  QQuickWindow::setTextRenderType(QQuickWindow::CurveTextRendering);
  // The controls of this application are fully styled by the qml layer. Without an explicit
  // style, the platform style is used, which draws its own hover and scroll visuals on top of
  // the custom ones. The basic style is the neutral style that leaves the controls untouched.
  QQuickStyle::setStyle("Basic");
  LOG_INFO("qml controls style: \"{}\"", QQuickStyle::name().toStdString());
}

///
///
auto load_qml_document(QQmlApplicationEngine& engine, QGuiApplication& app, const QString& url) -> void
{
  QObject::connect(
    &engine,
    &QQmlApplicationEngine::objectCreationFailed,
    &app,
    [](const QUrl& failed)
    {
      LOG_INFO("qml object creation failed: url: \"{}\"", failed.toString().toStdString());
      QCoreApplication::exit(EXIT_FAILURE);
    },
    Qt::QueuedConnection
  );
  engine.load(QUrl(url));
}

} // namespace aba
