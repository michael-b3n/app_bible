pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

///
/// Switch control for boolean value types.
///
ParamBase
{
  id: root

  // Properties
  topPadding: Metrics.spacingTiny
  leftPadding: root.titleWidth

  // Components
  contentItem: Switch
  {
    id: control

    // Properties
    // Note optional params may hold no value at all.
    checked: root.value === undefined ? false : root.value

    implicitWidth: root.availableWidth
    implicitHeight: root.titleHeight

    // Connections
    onClicked: { root.paramValueChanged(control.checked) }

    Connections
    {
      target: root
      function onValueChanged()
      {
        if(root.value !== undefined)
        {
          control.checked = root.value
        }
        else
        {
          // TODO display a warning
        }
      }
    }

    // Components
    indicator: Rectangle
    {
      // Properties
      width: 2 * height
      height: root.titleHeight
      x: Math.min(control.leftPadding, control.leftPadding + (control.availableWidth - width) / 3)
      y: control.topPadding + (control.availableHeight - height) / 2
      radius: height / 2
      color: control.checked ? Colors.selection : Colors.backgroundSolidDarker
      border.color: control.checked ? Colors.borderDarker : Colors.border

      // Components
      Rectangle
      {
        // Properties
        x: control.checked ? parent.width - width : 0
        width: parent.height
        height: parent.height
        radius: parent.height / 2
        color: control.down ? Colors.borderDarker : Colors.border
        border.color: control.checked ? (control.down ? Colors.greenDarker : Colors.green) : Colors.borderDarker
        border.width: Metrics.borderThick
      }
    }
  }
}
