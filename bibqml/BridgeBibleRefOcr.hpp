#pragma once

#include <bibstd/framework/runtime_uid.hpp>

#include <QObject>
#include <QPoint>
#include <QtQml/qqmlregistration.h>

namespace bibstd::presenter
{
class presenter_bible_ref_ocr;
} // namespace bibstd::presenter

namespace bibstd::signal
{
class connection_store;
} // namespace bibstd::signal

namespace bibqml
{

///
/// QML bridge for presenter_bible_ref_ocr.
/// Connects to presenter signals and exposes data via Q_PROPERTY.
///
class BridgeBibleRefOcr final : public QObject
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(bool visible MEMBER visible_ NOTIFY visibleChanged)
  Q_PROPERTY(QPoint cursorPosition MEMBER cursorPosition_ NOTIFY cursorPositionChanged)

public: // Structors
  explicit BridgeBibleRefOcr(bibstd::presenter::presenter_bible_ref_ocr& presenter, QObject* parent = nullptr);
  ~BridgeBibleRefOcr() noexcept override;

signals:
  void visibleChanged(bool visible);
  void cursorPositionChanged(const QPoint& cursorPosition);

private: // Variables
  bool visible_{false};
  QPoint cursorPosition_{0, 0};
  bibstd::framework::runtime_uid_type processId_{};
  std::unique_ptr<bibstd::signal::connection_store> connections_;
};

} // namespace bibqml
