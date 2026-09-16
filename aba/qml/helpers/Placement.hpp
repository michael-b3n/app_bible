#pragma once

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QtQmlIntegration/qqmlintegration.h>

namespace aba::qml
{

///
/// QML Placement singleton.
/// This class provides the placement math of the application windows.
/// All rects and points are given in screen coordinates.
///
class Placement final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

public: // Structors
  explicit Placement(QObject* parent = nullptr);
  ~Placement() noexcept override;

public: // Accessors
  ///
  /// \return geometry of the screen at the position, of the primary screen if there is none
  ///
  Q_INVOKABLE QRectF screenGeometryAt(const QPointF& position) const;

  ///
  /// \return geometry of the screen the center of the rect is on, of the primary screen if there is
  ///         none
  ///
  Q_INVOKABLE QRectF screenGeometryOf(const QRectF& rect) const;

  ///
  /// An empty rect stays empty, so an unknown area is not turned into a known one.
  /// \return rect grown by the amount on every side
  ///
  Q_INVOKABLE QRectF grown(const QRectF& source, qreal amount) const;

  ///
  /// \return rect moved onto the screen, keeping its size
  ///
  Q_INVOKABLE QRectF insideScreen(const QRectF& target, const QRectF& screen) const;

  ///
  /// \return square of the size centered at the position, moved onto the screen
  ///
  Q_INVOKABLE QRectF centeredSquare(const QPointF& center, qreal size, const QRectF& screen) const;

  ///
  /// Clips every border on its own, so that a border dragged beyond the screen stops
  /// there instead of pushing the opposite border along.
  /// \return clipped rect, at least of the minimal size
  ///
  Q_INVOKABLE QRectF clippedToScreen(const QRectF& target, const QRectF& screen, qreal minimalWidth, qreal minimalHeight) const;

  ///
  /// Moves the target onto the screen and then to the side of the blocked rect that keeps it there
  /// and is closest to its current position, so a blocked rect on the right of the screen moves the
  /// target to its left, one at the top moves it below and so on. The clearance is kept free around
  /// the blocked rect.
  /// \return moved rect, the target itself if the blocked rect is unknown or no side is free
  ///
  Q_INVOKABLE QRectF placedBeside(const QRectF& target, const QRectF& blocked, qreal clearance, const QRectF& screen) const;

  ///
  /// \return point on the border of the rect grown by the gap that is closest to the center of the
  ///         rect pointing at it
  ///
  Q_INVOKABLE QPointF borderPointTowards(const QRectF& rect, qreal gap, const QRectF& towards) const;

private: // Implementation
  QRectF besideRect(const QRectF& target, const QRectF& blocked, const QRectF& screen) const;
};

} // namespace aba::qml
