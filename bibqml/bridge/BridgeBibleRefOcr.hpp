#pragma once

#include <bibstd/framework/process_params.hpp>
#include <bibstd/signal/synchronized_executor.hpp>
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
  void referenceFound(const QString& bookId, int chapter, int verse);

public: // Modifiers
  ///
  /// Disconnect all signal connections.
  /// This will stop the frontend backend communication.
  ///
  void disconnect();

private: // Variables
  bool running_{false};
  QPoint cursorPosition_{0, 0};
  QString htmlPassage_{};

  const bibstd::workflow::workflow_hotkey::shared_sig_type findReferenceSig_{};
  bibstd::framework::process_id_type processId_{};
  bibstd::signal::synchronized_executor executor_;
};

} // namespace bibqml
