#include "BibQml/BridgeSettingBinder.hpp"

namespace bibqml
{

///
///
BridgeSettingBinder::BridgeSettingBinder(const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject(parent)
{
}

///
///
QString BridgeSettingBinder::test() const
{
  return QStringLiteral("BridgeSettingBinder singleton working!");
}

} // namespace BibQml
