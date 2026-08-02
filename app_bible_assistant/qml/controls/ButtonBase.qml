import QtQuick
import QtQuick.Controls

///
/// Base of all icon buttons.
/// Implementations shall add their content, the size
/// is expected to be set by the call site.
///
Button
{
  id: root

  // Properties
  implicitWidth: Metrics.controlHeight
  implicitHeight: Metrics.controlHeight
  padding: 0

  // Style
  background: Rectangle
  {
    // Properties
    color: root.pressed ? Colors.pressed : (root.hovered ? Colors.selection : "transparent")
    radius: Metrics.radiusSmall
  }
}
