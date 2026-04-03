#include "bibqml/BridgeSettings.hpp"

namespace bibqml
{

///
///
BridgeSettings::BridgeSettings(const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject(parent)
{
}

///
///
QString BridgeSettings::test() const
{
  return QStringLiteral("BridgeSettings singleton working!");
}

} // namespace bibqml
