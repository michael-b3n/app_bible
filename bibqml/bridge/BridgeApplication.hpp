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

  Q_PROPERTY(bool updateAvailable READ updateAvailable NOTIFY updateAvailableChanged FINAL)

  // Variables
  bool updateAvailable_{false};

public: // Structors
  explicit BridgeApplication(bibstd::util::non_owning_ptr<QObject> parent = nullptr);
  ~BridgeApplication() noexcept override;

public: // Accessors
  ///
  /// \return true if an update is downloaded and ready to be installed
  ///
  [[nodiscard]] bool updateAvailable() const;

public: // Modifiers
  ///
  /// Request the QML layer to show the main window.
  ///
  void requestShowWindow();

  ///
  /// Tell the QML layer that an update is downloaded and ready to be installed.
  ///
  void notifyUpdateAvailable();

  ///
  /// Request the application to install the available update and restart.
  ///
  Q_INVOKABLE void requestUpdate();

signals:
  void showWindowRequested();
  void updateAvailableChanged();
  void updateRequested();
};

} // namespace bibqml
