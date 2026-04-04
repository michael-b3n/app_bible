#pragma once

#include <QColor>
#include <QObject>
#include <QtQmlIntegration/qqmlintegration.h>

namespace aba::qml
{

///
/// QML Colors singleton.
/// This class provides application wide style colors.
///
class Colors final : public QObject
{
  Q_OBJECT
  QML_ELEMENT
  QML_SINGLETON

  Q_PROPERTY(QColor backgroundTransparent MEMBER backgroundTransparent_ NOTIFY backgroundTransparentChanged)
  Q_PROPERTY(QColor backgroundSolid MEMBER backgroundSolid_ NOTIFY backgroundSolidChanged)
  Q_PROPERTY(QColor border MEMBER border_ NOTIFY borderChanged)
  Q_PROPERTY(QColor text MEMBER text_ NOTIFY textChanged)
  Q_PROPERTY(QColor hover MEMBER hover_ NOTIFY hoverChanged)
  Q_PROPERTY(QColor pressed MEMBER pressed_ NOTIFY pressedChanged)

public: // Structors
  explicit Colors(QObject* parent = nullptr);
  ~Colors() noexcept override;

signals:
  void backgroundTransparentChanged();
  void backgroundSolidChanged();
  void borderChanged();
  void textChanged();
  void hoverChanged();
  void pressedChanged();

private: // Variables
  QColor backgroundTransparent_{"#3ad0d2e2"};
  QColor backgroundSolid_{"#3ad0d2ff"};
  QColor border_{"#adb9baff"};
  QColor text_{"#ffffff"};
  QColor hover_{"#69b2b4ff"};
  QColor pressed_{"#1e7c7dff"};
};

} // namespace aba::qml
