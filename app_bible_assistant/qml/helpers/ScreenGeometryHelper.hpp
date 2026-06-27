#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QtQmlIntegration/qqmlintegration.h>

namespace aba::qml
{

///
/// QML ScreenGeometryHelper singleton.
/// This class provides application wide helpers for screens.
///
class ScreenGeometryHelper final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

public: // Structors
  explicit ScreenGeometryHelper(QObject* parent = nullptr);
  ~ScreenGeometryHelper() noexcept override;

public: // Accessors
  ///
  /// Get the geometry of the screen at the specified position.
  /// On a multi monitor setup, this function returns the geometry of the monitor
  /// that contains the specified global position.
  /// \return geometry of the screen at the specified position
  /// in QML as `Qt.rect(x: ..., y: ..., width: ..., height: ...)`
  ///
  Q_INVOKABLE QRect screenGeometryAt(const QPoint& global_pos);
};

} // namespace aba::qml
