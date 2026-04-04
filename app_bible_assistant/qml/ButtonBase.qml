import QtQuick
import QtQuick.Controls

Button
{
  id: root

  implicitWidth: 100
  implicitHeight: 100
  padding: 0

  background: Rectangle
  {
    color: root.pressed ? Colors.pressed : (root.hovered ? Colors.hover : "transparent")
    radius: height * 0.1
  }
}
