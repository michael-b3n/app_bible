import QtQuick
import QtQuick.VectorImage

///
/// Tab button of the scripture tab. It doubles as the indicator of the reference search:
/// it spins while a search runs and rests on a check mark once it is over.
///
TabButtonBase
{
  id: root

  // Properties
  required property bool searchRunning

  // Components
  VectorImage
  {
    // Properties
    anchors.fill: parent
    visible: root.searchRunning
    source: Icons.loading
    preferredRendererType: VectorImage.CurveRenderer

    // Animations
    RotationAnimator on rotation
    {
      running: root.searchRunning && root.visible
      loops: Animation.Infinite
      from: 0
      to: 360
      duration: Metrics.durationLong
    }
  }

  VectorImage
  {
    // Properties
    anchors.fill: parent
    visible: !root.searchRunning
    source: Icons.checkMark
    preferredRendererType: VectorImage.CurveRenderer
  }
}
