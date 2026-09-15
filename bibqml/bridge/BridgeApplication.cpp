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
bool BridgeApplication::updateAvailable() const
{
  return updateAvailable_;
}

///
///
void BridgeApplication::requestShowWindow()
{
  QMetaObject::invokeMethod(this, [this]() { emit showWindowRequested(); }, Qt::QueuedConnection);
}

///
///
void BridgeApplication::notifyUpdateAvailable()
{
  QMetaObject::invokeMethod(
    this,
    [this]()
    {
      if(!updateAvailable_)
      {
        updateAvailable_ = true;
        emit updateAvailableChanged();
      }
    },
    Qt::QueuedConnection
  );
}

///
///
void BridgeApplication::requestUpdate()
{
  emit updateRequested();
}

} // namespace bibqml
