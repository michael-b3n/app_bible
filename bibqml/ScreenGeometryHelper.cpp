#include "bibqml/ScreenGeometryHelper.hpp"

#include <QGuiApplication>
#include <QScreen>

namespace bibqml
{

///
///
ScreenGeometryHelper::ScreenGeometryHelper(QObject* parent)
  : QObject(parent)
{
}

///
///
ScreenGeometryHelper::~ScreenGeometryHelper() noexcept = default;

///
///
QRect ScreenGeometryHelper::screenGeometryAt(const QPoint& global_pos)
{
  const auto screen = QGuiApplication::screenAt(global_pos);
  if(screen)
  {
    return screen->geometry();
  }
  return QRect(0, 0, 0, 0);
}

} // namespace bibqml
