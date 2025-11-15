#pragma once

#include <bibstd/util/non_owning_ptr.hpp>

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace bibqml
{

///
/// QML property binding for type-erased settings.
/// This class can bind to all supported type-erased setting types.
///
class SettingBinder : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

public: // Structors
  explicit SettingBinder(bibstd::util::non_owning_ptr<QObject> parent = nullptr);

public: // Invokable methods
  Q_INVOKABLE QString test() const;
};

} // namespace bibqml
