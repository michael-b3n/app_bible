import QtQuick
import QtQuick.VectorImage

ButtonBase
{
  id: root

  required property string svgSourceFirst
  required property string svgSourceSecond
  property bool toggled: false

  onClicked: { root.toggled = !root.toggled }

  VectorImage
  {
    visible: !root.toggled
    anchors.fill: parent
    source: root.svgSourceFirst
    preferredRendererType: VectorImage.CurveRenderer
  }

  VectorImage
  {
    visible: root.toggled
    anchors.fill: parent
    source: root.svgSourceSecond
    preferredRendererType: VectorImage.CurveRenderer
  }
}
