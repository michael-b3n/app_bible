#pragma once

#include <bibstd/util/non_owning_ptr.hpp>

#include <QObject>
#include <QString>
#include <QtQmlIntegration/qqmlintegration.h>

namespace bibqml
{

///
/// QML logger. This class will provide backend logging functionality to QML.
///
class BridgeLogger : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

public: // Structors
  explicit BridgeLogger(bibstd::util::non_owning_ptr<QObject> parent = nullptr);

public: // Invokable methods
  Q_INVOKABLE void log_debug(const QString& message) const;
  Q_INVOKABLE void log_info(const QString& message) const;
  Q_INVOKABLE void log_warning(const QString& message) const;
  Q_INVOKABLE void log_error(const QString& message) const;
};

} // namespace bibqml
