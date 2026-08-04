#include "bibqml/bridge/BridgeApplication.hpp"

#include <QMetaObject>

namespace bibqml
{

///
///
BridgeApplication::BridgeApplication(const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject{parent}
{
}

///
///
BridgeApplication::~BridgeApplication() noexcept = default;

///
///
void BridgeApplication::requestShowWindow()
{
  QMetaObject::invokeMethod(this, [this]() { emit showWindowRequested(); }, Qt::QueuedConnection);
}

} // namespace bibqml
