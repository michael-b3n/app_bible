#pragma once

#include "bibqml/util/SettingValue.hpp"

#include <bibstd/signal/synchronized_executor.hpp>
#include <bibstd/util/non_owning_ptr.hpp>

#include <QObject>
#include <QQmlParserStatus>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>
#include <QVariant>

#include <optional>

namespace bibqml
{

///
/// QML setting binding.
/// This element binds a QML property to a setting of the settings workflow. If no setting
/// exists for the given path, a new one is created from the given default value. This allows
/// the QML layer to declare its own settings, e.g. style colors, without the backend knowing
/// about them. Settings declared this way are persisted like any other setting.
///
/// Usage:
/// \code
///   SettingBinding { id: accentColor; path: "ui.color.accent"; defaultValue: "#1e301e" }
///   ...
///   color: accentColor.value
/// \endcode
///
/// `path` and `defaultValue` describe which setting this binding operates on. They are
/// constant, they can only be set while this element is initialized. Only `value` changes
/// at runtime, in both directions.
///
/// A setting that is created by this binding is created with the value type of `defaultValue`.
/// Supported are boolean, integral, floating point, string and color values. Colors are stored
/// as "#AARRGGBB" strings. Settings declared from QML are unbound, they have no validator.
/// Reading and writing `value` is defined by the value type of the bound setting, therefore a
/// binding can also be attached to a setting of any value type the backend declared.
///
/// \note This type must not be final since it is instantiated by the QML engine.
///
class SettingBinding
  : public QObject
  , public QQmlParserStatus
{
  Q_OBJECT
  QML_ELEMENT
  Q_INTERFACES(QQmlParserStatus)

  Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged FINAL)
  Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue NOTIFY defaultValueChanged FINAL)
  Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged FINAL)
  Q_PROPERTY(bool bound READ bound NOTIFY boundChanged FINAL)

  // Variables
  QString path_{};
  QVariant defaultValue_{};
  std::optional<SettingVariantType> setting_{};
  bool complete_{false};
  bibstd::signal::synchronized_executor executor_{};

public: // Structors
  explicit SettingBinding(bibstd::util::non_owning_ptr<QObject> parent = nullptr);
  ~SettingBinding() noexcept override;

public: // Overrides
  void classBegin() override;
  void componentComplete() override;

public: // Accessors
  ///
  /// \return path of the bound setting
  ///
  [[nodiscard]] QString path() const;

  ///
  /// \return default value the setting is created with
  ///
  [[nodiscard]] QVariant defaultValue() const;

  ///
  /// \return current value of the setting, the default value if this binding is not bound
  ///
  [[nodiscard]] QVariant value() const;

  ///
  /// \return true if this binding is bound to a setting, false otherwise
  ///
  [[nodiscard]] bool bound() const;

public: // Setters
  ///
  /// Set the path of the setting this binding operates on.
  /// \note The path is constant, it can only be set while this element is initialized.
  ///
  void setPath(const QString& path);

  ///
  /// Set the default value a setting that does not exist yet is created with.
  /// \note The default value is constant, it can only be set while this element is initialized.
  ///
  void setDefaultValue(const QVariant& value);

  ///
  /// Write the value to the bound setting. The value is validated by the setting, therefore
  /// the value of the setting may differ from the value written.
  ///
  void setValue(const QVariant& value);

signals:
  void pathChanged();
  void defaultValueChanged();
  void valueChanged();
  void boundChanged();

private: // Implementation
  ///
  /// Bind to the setting of the path, creating it if it does not exist yet.
  /// This is called once, when the initialization of this element is complete.
  ///
  void bind();
};

} // namespace bibqml
