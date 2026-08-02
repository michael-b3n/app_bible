import QtQuick
import QtQuick.Controls

///
/// Base of all tab buttons.
/// Implementations shall add their content, the size
/// is expected to be set by the call site.
///
TabButton
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
    // Note the pressed state takes precedence over the checked state,
    // otherwise pressing the current tab gives no feedback at all.
    color:
    {
      if(root.pressed) { return Colors.pressed }
      if(root.hovered) { return Colors.selection }
      if(root.checked) { return Colors.backgroundSolidDarker }
      return "transparent"
    }
    radius: Metrics.radiusSmall
  }
}
