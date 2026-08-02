import QtQuick
import QtQuick.VectorImage

TabButtonBase
{
  id: root

  // Properties
  required property bool runningState

  // Components
  VectorImage
  {
    visible: root.runningState
    anchors.fill: parent
    source: Icons.loading
    preferredRendererType: VectorImage.CurveRenderer

    RotationAnimator on rotation
    {
      running: root.runningState && root.visible
      loops: Animation.Infinite
      from: 0
      to: 360
      duration: Metrics.durationLong
    }
  }

  VectorImage
  {
    visible: !root.runningState
    anchors.fill: parent
    source: Icons.checkMark
    preferredRendererType: VectorImage.CurveRenderer
  }
}
