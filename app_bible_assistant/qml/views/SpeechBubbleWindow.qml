import QtQuick

///
/// Transparent window covering a whole screen, drawing the speech bubble shape around the
/// window whose rect it is given. It never takes any input.
///
Window
{
  id: root

  // Properties
  required property rect screenGeometry
  required property rect bubbleRect
  required property point tailPosition
  required property bool tailVisible
  required property bool shown

  color: "transparent"
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
  visible: shape.opacity > 0
  x: root.screenGeometry.x
  y: root.screenGeometry.y
  width: root.screenGeometry.width
  height: root.screenGeometry.height

  // Components
  SpeechBubbleShape
  {
    id: shape

    // Properties
    opacity: root.shown ? 1 : 0
    radius: Metrics.radiusLarge
    tailVisible: root.tailVisible
    tailPositionX: root.tailPosition.x - root.x
    tailPositionY: root.tailPosition.y - root.y
    offsetToTailX: root.bubbleRect.x - root.tailPosition.x
    offsetToTailY: root.bubbleRect.y - root.tailPosition.y
    bubbleWidth: root.bubbleRect.width
    bubbleHeight: root.bubbleRect.height
    strokeColor: Colors.border
    fillColor: Colors.backgroundTransparent

    // Animations
    Behavior on opacity
    {
      NumberAnimation
      {
        duration: Metrics.durationShort
        easing.type: Easing.InOutQuad
      }
    }
  }
}
