#include "BibQml/SettingBinder.hpp"

namespace bibqml
{

///
///
SettingBinder::SettingBinder(const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject(parent)
{
}

///
///
QString SettingBinder::test() const
{
  return QStringLiteral("SettingBinder singleton working!");
}

} // namespace BibQml
