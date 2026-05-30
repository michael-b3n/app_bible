#include "ScreenGeometryHelper.hpp"

#include <QGuiApplication>
#include <QScreen>

namespace aba::qml
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

} // namespace aba::qml
