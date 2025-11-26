import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.VectorImage

Button
{
  id: root

  required property string svgSource

  implicitWidth: 100
  implicitHeight: 100
  padding: 0

  background: Rectangle
  {
    color: root.pressed ? Colors.pressed : (root.hovered ? Colors.hover : "transparent")
    radius: height * 0.1
  }

  VectorImage
  {
    anchors.fill: parent
    source: root.svgSource
    preferredRendererType: VectorImage.CurveRenderer
  }
}
