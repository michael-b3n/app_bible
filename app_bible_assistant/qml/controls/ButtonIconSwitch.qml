import QtQuick
import QtQuick.VectorImage

///
/// Icon button toggling between two svg icons.
///
ButtonBase
{
  id: root

  // Properties
  required property string svgSourceFirst
  required property string svgSourceSecond
  property bool toggled: false

  // Connections
  onClicked: { root.toggled = !root.toggled }

  // Components
  VectorImage
  {
    // Properties
    visible: !root.toggled
    anchors.fill: parent
    source: root.svgSourceFirst
    preferredRendererType: VectorImage.CurveRenderer
  }

  VectorImage
  {
    // Properties
    visible: root.toggled
    anchors.fill: parent
    source: root.svgSourceSecond
    preferredRendererType: VectorImage.CurveRenderer
  }
}
