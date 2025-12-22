import QtQuick
import QtQuick.Shapes

Item
{
  id: root

  required property int radius
  required property int tailPositionX
  required property int tailPositionY
  required property int offsetToTailX
  required property int offsetToTailY

  required property int bubbleWidth
  required property int bubbleHeight
  readonly property int bubbleX: tailPositionX + offsetToTailX
  readonly property int bubbleY: tailPositionY + offsetToTailY

  readonly property int strokeWidth: 2
  readonly property int margin: 2 * strokeWidth

  readonly property int maxTailLength: Math.min(bubbleWidth, bubbleHeight)
  readonly property double tailWidth: radius

  implicitHeight: 100
  implicitWidth: 200

  Shape
  {
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    ShapePath
    {
      id: bubblePath

      readonly property int lineLeftX: root.bubbleX + root.radius
      readonly property int lineRightX: root.bubbleX + root.bubbleWidth - root.radius
      readonly property int lineTopY: root.bubbleY + root.radius
      readonly property int lineBottomY: root.bubbleY + root.bubbleHeight - root.radius

      readonly property bool tailUp: root.tailPositionY < root.bubbleY
      readonly property bool tailDown: root.tailPositionY > root.bubbleY + root.bubbleHeight
      readonly property bool tailLeft: !tailUp && !tailDown && root.tailPositionX < root.bubbleX
      readonly property bool tailRight: !tailUp && !tailDown && root.tailPositionX > root.bubbleX + root.bubbleWidth

      readonly property int tailProjectionLeftX: Math.max(lineLeftX, Math.min(root.tailPositionX - root.tailWidth / 2, lineRightX - root.tailWidth))
      readonly property int tailProjectionRightX: Math.min(lineRightX, Math.max(root.tailPositionX + root.tailWidth / 2, lineLeftX + root.tailWidth))
      readonly property int tailProjectionTopY: Math.max(lineTopY, Math.min(root.tailPositionY - root.tailWidth / 2, lineBottomY - root.tailWidth))
      readonly property int tailProjectionBottomY: Math.min(lineBottomY, Math.max(root.tailPositionY + root.tailWidth / 2, lineTopY + root.tailWidth))

      strokeColor: Colors.border
      strokeWidth: root.strokeWidth /*border width*/
      fillColor: Colors.backgroundTransparent
      pathHints: ShapePath.PathConvex | ShapePath.PathSolid | ShapePath.PathNonIntersecting | ShapePath.PathFillOnRight
      startX: root.bubbleX + root.radius /*top left corner next to radius*/
      startY: root.bubbleY /*to left corner*/

      // Top side
      PathLine
      {
        x: bubblePath.tailProjectionLeftX
        y: root.bubbleY
      }
      PathLine
      {
        x: bubblePath.tailUp ? root.tailPositionX : bubblePath.tailProjectionLeftX
        y: bubblePath.tailUp ? root.tailPositionY : root.bubbleY
      }
      PathLine
      {
        x: bubblePath.tailProjectionRightX
        y: root.bubbleY
      }
      PathLine
      {
        x: root.bubbleX + root.bubbleWidth - root.radius
        y: root.bubbleY
      }
      // Top-right corner
      PathArc
      {
        x: root.bubbleX + root.bubbleWidth
        y: root.bubbleY + root.radius
        radiusX: root.radius
        radiusY: root.radius
        direction: PathArc.Clockwise
      }
      // Right side
      PathLine
      {
        x: root.bubbleX + root.bubbleWidth
        y: bubblePath.tailProjectionTopY
      }
      PathLine
      {
        x: bubblePath.tailRight ? root.tailPositionX : root.bubbleX + root.bubbleWidth
        y: bubblePath.tailRight ? root.tailPositionY : bubblePath.tailProjectionTopY
      }
      PathLine
      {
        x: root.bubbleX + root.bubbleWidth
        y: bubblePath.tailProjectionBottomY
      }
      PathLine
      {
        x: root.bubbleX + root.bubbleWidth
        y: root.bubbleY + root.bubbleHeight - root.radius
      }
      // Bottom-right corner
      PathArc
      {
        x: root.bubbleX + root.bubbleWidth - root.radius
        y: root.bubbleY + root.bubbleHeight
        radiusX: root.radius
        radiusY: root.radius
        direction: PathArc.Clockwise
      }
      // Bottom side
      PathLine
      {
        x: bubblePath.tailProjectionRightX
        y: root.bubbleY + root.bubbleHeight
      }
      PathLine
      {
        x: bubblePath.tailDown ? root.tailPositionX : bubblePath.tailProjectionRightX
        y: bubblePath.tailDown ? root.tailPositionY : root.bubbleY + root.bubbleHeight
      }
      PathLine
      {
        x: bubblePath.tailProjectionLeftX
        y: root.bubbleY + root.bubbleHeight
      }
      PathLine
      {
        x: root.bubbleX + root.radius
        y: root.bubbleY + root.bubbleHeight
      }
      // Bottom-left corner
      PathArc
      {
        x: root.bubbleX
        y: root.bubbleY + root.bubbleHeight - root.radius
        radiusX: root.radius
        radiusY: root.radius
        direction: PathArc.Clockwise
      }
      // Left side
      PathLine
      {
        x: root.bubbleX
        y: bubblePath.tailProjectionBottomY
      }
      PathLine
      {
        x: bubblePath.tailLeft ? root.tailPositionX : root.bubbleX
        y: bubblePath.tailLeft ? root.tailPositionY : bubblePath.tailProjectionBottomY
      }
      PathLine
      {
        x: root.bubbleX
        y: bubblePath.tailProjectionTopY
      }
      PathLine
      {
        x: root.bubbleX
        y: root.bubbleY + root.radius
      }
      // Top-left corner
      PathArc
      {
        x: root.bubbleX + root.radius
        y: root.bubbleY
        radiusX: root.radius
        radiusY: root.radius
        direction: PathArc.Clockwise
      }
    }
  }
}
