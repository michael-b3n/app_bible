#pragma once

#include <bibstd/framework/runtime_uid.hpp>

#include <QObject>
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
  Q_PROPERTY(int cursorX MEMBER cursorX_ NOTIFY cursorXChanged)
  Q_PROPERTY(int cursorY MEMBER cursorY_ NOTIFY cursorYChanged)

public: // Structors
  explicit BridgeBibleRefOcr(bibstd::presenter::presenter_bible_ref_ocr& presenter, QObject* parent = nullptr);
  ~BridgeBibleRefOcr() noexcept override;

signals:
  void visibleChanged();
  void cursorXChanged();
  void cursorYChanged();

private: // Variables
  bool visible_{false};
  int cursorX_{0};
  int cursorY_{0};
  bibstd::framework::runtime_uid_type processId_{};
  std::unique_ptr<bibstd::signal::connection_store> connections_;
};

} // namespace bibqml
