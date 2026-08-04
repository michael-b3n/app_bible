#pragma once

#include <bibstd/util/non_owning_ptr.hpp>

#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace bibqml
{

///
/// QML bridge for application wide requests.
/// This forwards requests that reach the application outside of the QML layer, e.g. from the
/// system tray, to the QML layer. A request may be made from any thread.
///
class BridgeApplication : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_UNCREATABLE("BridgeApplication is created by the application")

public: // Structors
  explicit BridgeApplication(bibstd::util::non_owning_ptr<QObject> parent = nullptr);
  ~BridgeApplication() noexcept override;

public: // Modifiers
  ///
  /// Request the QML layer to show the main window.
  ///
  void requestShowWindow();

signals:
  void showWindowRequested();
};

} // namespace bibqml
