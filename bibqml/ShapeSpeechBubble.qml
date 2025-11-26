import QtQuick
import QtQuick.Shapes

Item
{
  id: root

  required property int tailPositionX
  required property bool upwards

  readonly property double tailHeight: Math.min(root.height, root.width) * 0.2
  readonly property double tailWidth: tailHeight
  readonly property double radius: tailHeight
  readonly property int innerX: radius / 2
  readonly property int innerY: root.upwards ? radius / 2 : tailHeight + radius / 2
  readonly property int innerWidth: width - radius
  readonly property int innerHeight: height - radius - tailHeight

  implicitHeight: 100
  implicitWidth: 200

  Shape
  {
    anchors.fill: parent
    preferredRendererType: Shape.CurveRenderer

    ShapePath
    {
      strokeColor: Colors.border
      strokeWidth: 2
      fillColor: Colors.backgroundTransparent
      pathHints: ShapePath.PathConvex | ShapePath.PathSolid | ShapePath.PathNonIntersecting
      startX: root.tailPositionX
      startY: root.upwards ? root.height : 0

      PathLine
      {
        readonly property int tailLeftX: Math.max(root.radius, root.tailPositionX - root.tailWidth / 2)
        x: Math.min(tailLeftX, root.width - root.radius - root.tailWidth)
        y: root.upwards ? root.height - root.tailHeight : root.tailHeight
      }
      PathLine
      {
        x: root.radius
        y: root.upwards ? root.height - root.tailHeight : root.tailHeight
      }
      PathArc
      {
        x: 0
        y: root.upwards ? root.height - root.tailHeight - root.radius : root.tailHeight + root.radius
        radiusX: root.radius
        radiusY: root.radius
        direction: root.upwards ? PathArc.Clockwise : PathArc.Counterclockwise
      }
      PathLine
      {
        x: 0
        y: root.upwards ? root.radius : root.height - root.radius
      }
      PathArc
      {
        x: root.radius
        y: root.upwards ? 0 : root.height
        radiusX: root.radius
        radiusY: root.radius
        direction: root.upwards ? PathArc.Clockwise : PathArc.Counterclockwise
      }
      PathLine
      {
        x: root.width - root.radius
        y: root.upwards ? 0 : root.height
      }
      PathArc
      {
        x: root.width
        y: root.upwards ? root.radius : root.height - root.radius
        radiusX: root.radius
        radiusY: root.radius
        direction: root.upwards ? PathArc.Clockwise : PathArc.Counterclockwise
      }
      PathLine
      {
        x: root.width
        y: root.upwards ? root.height - root.tailHeight - root.radius : root.tailHeight + root.radius
      }
      PathArc
      {
        x: root.width - root.radius
        y: root.upwards ? root.height - root.tailHeight : root.tailHeight
        radiusX: root.radius
        radiusY: root.radius
        direction: root.upwards ? PathArc.Clockwise : PathArc.Counterclockwise
      }
      PathLine
      {
        readonly property int tailRightX: Math.min(root.width - root.radius, root.tailPositionX + root.tailWidth / 2)
        x: Math.max(tailRightX, root.radius + root.tailWidth)
        y: root.upwards ? root.height - root.tailHeight : root.tailHeight
      }
      PathLine
      {
        x: root.tailPositionX
        y: root.upwards ? root.height : 0
      }
    }
  }
}
