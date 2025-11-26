#include "bibqml/ScreenHelper.hpp"

#include <QGuiApplication>
#include <QScreen>

namespace bibqml
{

///
///
ScreenHelper::ScreenHelper(QObject* parent)
  : QObject(parent)
{
}

///
///
ScreenHelper::~ScreenHelper() noexcept = default;

///
///
QRect ScreenHelper::screenGeometryAt(const QPoint& global_pos)
{
  const auto screen = QGuiApplication::screenAt(global_pos);
  if(screen)
  {
    return screen->geometry();
  }
  return QRect(0, 0, 0, 0);
}

} // namespace bibqml
