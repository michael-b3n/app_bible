#pragma once

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QtQmlIntegration/qqmlintegration.h>

namespace bibqml
{

///
/// QML ScreenHelper singleton.
/// This class provides application wide helpers for screens.
///
class ScreenHelper final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

public: // Structors
  explicit ScreenHelper(QObject* parent = nullptr);
  ~ScreenHelper() noexcept override;

public: // Accessors
  ///
  /// Get the geometry of the screen at the specified position.
  /// On a multi monitor setup, this function returns the geometry of the monitor
  /// that contains the specified global position.
  /// \param global_pos Global position
  /// \return geometry of the screen at the specified position
  /// in QML as `Qt.rect(x: ..., y: ..., width: ..., height: ...)`
  ///
  Q_INVOKABLE QRect screenGeometryAt(const QPoint& global_pos);
};

} // namespace bibqml
