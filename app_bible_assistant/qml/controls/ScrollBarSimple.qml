pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

ScrollBar
{
  id: control

  // Property
  policy: ScrollBar.AsNeeded

  // Components
  background: Rectangle
  {
    width: Metrics.controlHeight / 3
    implicitWidth: Metrics.controlHeight / 3
    color: Colors.backgroundSolid
    radius: Metrics.radiusMedium
    opacity: control.active ? 1.0 : 0.0

    Behavior on opacity { NumberAnimation { duration: 200 } }
  }
  contentItem: Rectangle
  {
    width: Metrics.controlHeight / 4
    implicitWidth: Metrics.controlHeight / 4
    radius: Metrics.radiusMedium
    color: control.pressed ? Colors.pressed : Colors.border
    opacity: control.active ? 1.0 : 0.0

    Behavior on opacity { NumberAnimation { duration: 200 } }
  }
}
