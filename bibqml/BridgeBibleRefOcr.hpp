#pragma once

#include <bibstd/framework/process_params.hpp>
#include <bibstd/signal/scoped_connection_guard.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>

#include <QObject>
#include <QPoint>
#include <qtmetamacros.h>
#include <QtQml/qqmlregistration.h>

namespace bibstd::workflow
{
// Forward declaration
class workflow_bible_ref_ocr;
} // namespace bibstd::workflow

namespace bibstd::signal
{
class connection_store;
} // namespace bibstd::signal

namespace bibqml
{

///
/// QML bridge for workflow_bible_ref_ocr.
/// Connects to workflow signals and exposes data via Q_PROPERTY.
///
class BridgeBibleRefOcr final : public QObject
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(bool running MEMBER running_ NOTIFY runningChanged)
  Q_PROPERTY(QPoint cursorPosition MEMBER cursorPosition_ NOTIFY cursorPositionChanged)
  Q_PROPERTY(QString htmlPassage MEMBER htmlPassage_ NOTIFY htmlPassageChanged)
  Q_PROPERTY(int htmlPassageBeginIndex MEMBER htmlPassageBeginIndex_ NOTIFY htmlPassageBeginIndexChanged)

public: // Structors
  explicit BridgeBibleRefOcr(
    std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr> workflow_bible_ref_ocr,
    std::shared_ptr<bibstd::workflow::workflow_hotkey> workflow_hotkey,
    QObject* parent = nullptr
  );
  ~BridgeBibleRefOcr() noexcept override;

signals:
  void runningChanged(bool running);
  void cursorPositionChanged(const QPoint& cursorPosition);
  void htmlPassageChanged(const QString& htmlPassage);
  void htmlPassageBeginIndexChanged(int htmlPassageBeginIndex);

public: // Modifiers
  ///
  /// Disconnect all signal connections.
  /// This will stop the frontend backend communication.
  ///
  auto disconnect() -> void;

private: // Implementation
  auto onFindReference() -> void;

private: // Variables
  bool running_{false};
  QPoint cursorPosition_{0, 0};
  QString htmlPassage_{};
  int htmlPassageBeginIndex_{0};

  const bibstd::workflow::workflow_hotkey::shared_sig_type findReferenceSig_{};
  bibstd::framework::process_id_type processId_{};
  std::unique_ptr<bibstd::signal::scoped_connection_guard> connections_;
};

} // namespace bibqml
