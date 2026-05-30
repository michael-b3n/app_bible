#pragma once

#include <bibstd/signal/synchronized_executor.hpp>

#include <QObject>
#include <qtmetamacros.h>
#include <QtQml/qqmlregistration.h>

namespace bibstd::workflow
{
// Forward declaration
class workflow_scripture;
} // namespace bibstd::workflow

namespace bibqml
{

///
/// QML bridge for workflow_scripture.
///
class BridgeScripture final : public QObject
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(bool test MEMBER test_ NOTIFY testChanged)

public: // Structors
  explicit BridgeScripture(std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture, QObject* parent = nullptr);
  ~BridgeScripture() noexcept override;

signals:
  void testChanged();

public: // Modifiers
  ///
  /// Disconnect all signal connections.
  /// This will stop the frontend backend communication.
  ///
  auto disconnect() -> void;

private: // Implementation

private: // Variables
  bool test_{false};
  std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture_;
  bibstd::signal::synchronized_executor executor_;
};

} // namespace bibqml
