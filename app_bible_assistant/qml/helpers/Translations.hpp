#pragma once

#include "src/pretty_names.hpp"

#include <bibstd/util/non_owning_ptr.hpp>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlengine.h>
#include <QtQmlIntegration/qqmlintegration.h>

namespace aba::qml
{

///
/// QML Translations singleton.
/// This class provides the pretty name of any identifier the application displays, e.g. the
/// path and the values of a setting, or the text of a button. Keys are plain strings, the
/// meaning of a key is defined by the caller only. The backend does not know about pretty
/// names, it only deals with identifiers, therefore all displayed identifiers are translated
/// here.
/// This class knows nothing about where the pretty names or the language come from. The
/// instance is created and owned by the application, which also provides the language.
/// The QML engine only accesses it as a singleton, it must outlive the QML engine.
/// \note Bindings must read `language` to be reevaluated on a language change. All invokable
/// methods accept the language as last argument for this purpose.
///
class Translations final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QString language READ language NOTIFY languageChanged FINAL)
  Q_PROPERTY(QStringList availableLanguages READ availableLanguages CONSTANT FINAL)

  // Variables
  const pretty_names names_;
  QString language_;

public: // Static interface
  ///
  /// Access the instance the QML layer operates on.
  /// \return the instance, nullptr if no instance exists
  ///
  [[nodiscard]] static bibstd::util::non_owning_ptr<Translations> instance();

  ///
  /// QML singleton factory. The returned instance stays owned by the application.
  /// \return the instance, nullptr if no instance exists
  ///
  static Translations* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

public: // Structors
  explicit Translations(pretty_names names, bibstd::util::non_owning_ptr<QObject> parent = nullptr);
  ~Translations() noexcept override;

public: // Accessors
  ///
  /// \return language the pretty names are displayed in
  ///
  [[nodiscard]] QString language() const;

  ///
  /// \return all languages the application provides pretty names for
  ///
  [[nodiscard]] QStringList availableLanguages() const;

public: // Setters
  ///
  /// Set the language the pretty names are displayed in. Unknown languages are ignored.
  ///
  void setLanguage(const QString& language);

public: // Methods
  ///
  /// Get the pretty name of a key, e.g. the path of a setting or the name of a button.
  /// \return pretty name, the key itself if no pretty name is available
  ///
  Q_INVOKABLE QString name(const QString& key, const QString& language = {}) const;

signals:
  void languageChanged();
};

} // namespace aba::qml
