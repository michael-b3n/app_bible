import QtQuick
import QtQuick.VectorImage

///
/// Icon button showing one of two svg icons.
/// The toggled state is owned by the call site, the button only reports the click.
///
ButtonBase
{
  id: root

  // Properties
  required property string svgSourceFirst
  required property string svgSourceSecond
  property bool toggled: false

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
