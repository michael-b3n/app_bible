#pragma once

#include "bibqml/bridge/SettingBinding.hpp"

#include <bibstd/util/non_owning_ptr.hpp>

#include <QObject>
#include <QString>
#include <QtQml/qqmlengine.h>
#include <QtQmlIntegration/qqmlintegration.h>
#include <QVariant>

#include <map>
#include <memory>

// Forward declarations
namespace bibstd::workflow
{
class workflow_settings;
} // namespace bibstd::workflow

namespace bibqml
{

///
/// QML settings registry.
/// This provides the settings workflow all setting bindings operate on and creates the
/// bindings the QML layer uses. The instance is created and owned by the application, the
/// QML engine only accesses it as a singleton. It must be created before the QML engine
/// loads a component that requests a setting binding, and it must outlive the QML engine.
/// \see SettingBinding
///
class BridgeSettings final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  // Variables
  const std::shared_ptr<bibstd::workflow::workflow_settings> workflowSettings_;
  std::map<QString, std::unique_ptr<SettingBinding>> bindings_{};

public: // Static interface
  ///
  /// Access the instance the QML layer operates on.
  /// \return the instance, nullptr if no instance exists
  ///
  [[nodiscard]] static bibstd::util::non_owning_ptr<BridgeSettings> instance();

  ///
  /// QML singleton factory. The returned instance stays owned by the application.
  /// \return the instance, nullptr if no instance exists
  ///
  static BridgeSettings* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

public: // Structors
  explicit BridgeSettings(
    std::shared_ptr<bibstd::workflow::workflow_settings> workflowSettings,
    bibstd::util::non_owning_ptr<QObject> parent = nullptr
  );
  ~BridgeSettings() noexcept override;

public: // Accessors
  ///
  /// Access the settings workflow all setting bindings operate on.
  /// \return settings workflow
  ///
  [[nodiscard]] auto workflowSettings() const -> const std::shared_ptr<bibstd::workflow::workflow_settings>&;

  ///
  /// Check if a setting with the specified path exists.
  /// \return true if the setting exists, false otherwise
  ///
  Q_INVOKABLE bool contains(const QString& path) const;

public: // Modifiers
  ///
  /// Access the setting binding of the specified path, creating it on first access. The
  /// setting itself is created from the default value if it does not exist yet. All callers
  /// of a path share one binding, therefore the default value of the first call wins. The
  /// returned binding is owned by this registry and stays valid as long as it exists.
  /// \return setting binding, nullptr if no binding could be created
  ///
  Q_INVOKABLE SettingBinding* binding(const QString& path, const QVariant& defaultValue);
};

} // namespace bibqml
