import QtQuick
import QtQuick.VectorImage

TabButtonBase
{
  id: root

  required property bool runningState

  VectorImage
  {
    visible: root.runningState
    anchors.fill: parent
    source: "qrc:/qt/qml/ui/qml/res/loading.svg"
    preferredRendererType: VectorImage.CurveRenderer

    RotationAnimator on rotation
    {
      running: root.runningState
      loops: Animation.Infinite
      from: 0;
      to: 360;
      duration: 3000
    }
  }

  VectorImage
  {
    visible: !root.runningState
    anchors.fill: parent
    source: "qrc:/qt/qml/ui/qml/res/check_mark.svg"
    preferredRendererType: VectorImage.CurveRenderer
  }
}
