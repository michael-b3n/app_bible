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

  // Geometry the bubble is drawn with, it only follows the requested one while the bubble is shown
  property rect currentBubbleRect: Qt.rect(0, 0, 0, 0)
  property point currentTailPosition: Qt.point(0, 0)

  color: "transparent"
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
  visible: shape.opacity > 0
  x: root.screenGeometry.x
  y: root.screenGeometry.y
  width: root.screenGeometry.width
  height: root.screenGeometry.height

  // Connections
  onBubbleRectChanged: { root.takeGeometry() }
  onTailPositionChanged: { root.takeGeometry() }
  onShownChanged: { root.takeGeometry() }

  // Components
  SpeechBubbleShape
  {
    id: shape

    // Properties
    opacity: root.shown ? 1 : 0
    radius: Metrics.radiusLarge
    tailVisible: root.tailVisible
    tailPositionX: root.currentTailPosition.x - root.x
    tailPositionY: root.currentTailPosition.y - root.y
    offsetToTailX: root.currentBubbleRect.x - root.currentTailPosition.x
    offsetToTailY: root.currentBubbleRect.y - root.currentTailPosition.y
    bubbleWidth: root.currentBubbleRect.width
    bubbleHeight: root.currentBubbleRect.height
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

  // Functions
  ///
  /// Takes over the geometry the bubble shall be drawn with. A bubble that is not shown keeps the
  /// geometry it has: it fades out instead of disappearing at once, and the bubble left on the
  /// screen must not jump onto the window and the reference of the next search.
  ///
  function takeGeometry()
  {
    if(!root.shown)
    {
      return
    }
    /*no binding*/ root.currentBubbleRect = root.bubbleRect
    /*no binding*/ root.currentTailPosition = root.tailPosition
  }
}
