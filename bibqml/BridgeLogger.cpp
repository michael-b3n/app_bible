#include "BibQml/BridgeLogger.hpp"

#include <bibstd/util/log.hpp>

namespace bibqml
{

///
///
BridgeLogger::BridgeLogger(const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject(parent)
{
}

///
///
void BridgeLogger::log_debug(const QString& message) const
{
  LOG_DEBUG("QML: {}", message.toStdString());
}

///
///
void BridgeLogger::log_info(const QString& message) const
{
  LOG_INFO("QML: {}", message.toStdString());
}

///
///
///
void BridgeLogger::log_warning(const QString& message) const
{
  LOG_WARN("QML: {}", message.toStdString());
}

///
///
void BridgeLogger::log_error(const QString& message) const
{
  LOG_ERROR("QML: {}", message.toStdString());
}

} // namespace bibqml
