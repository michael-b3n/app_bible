import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.VectorImage

Button
{
  id: root

  required property string svgSourceFirst
  required property string svgSourceSecond
  property bool toggled: false

  implicitWidth: 100
  implicitHeight: 100
  padding: 0

  onClicked:
  {
    root.toggled = !root.toggled
  }

  background: Rectangle
  {
    color: root.pressed ? Colors.pressed : (root.hovered ? Colors.hover : "transparent")
    radius: height * 0.1
  }

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
