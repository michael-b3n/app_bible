import QtQuick

Item
{
  id: root

  required property bool expandable
  required property int expandAreaWidth
  required property bool movable

  readonly property bool containsMouse:
    center.containsMouse ||
    topLeft.containsMouse ||
    topRight.containsMouse ||
    bottomLeft.containsMouse ||
    bottomRight.containsMouse

  implicitHeight: 100
  implicitWidth: 200

  signal pressed(mouse: MouseEvent)
  signal released(mouse: MouseEvent)
  signal expandRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)
  signal moveRequested(deltaX: int, deltaY: int)

  MouseArea
  {
    id: center

    property int clickX: 0
    property int clickY: 0

    hoverEnabled: true
    anchors.fill: parent

    onPressed: (mouse) =>
    {
      clickX = mouse.x
      clickY = mouse.y
      root.pressed(mouse)
    }
    onReleased: (mouse) => { root.released(mouse) }
    onPositionChanged: (mouse) =>
    {
      if(root.movable && pressed)
      {
        root.moveRequested(mouse.x - clickX, mouse.y - clickY)
      }
    }
  }

  MouseAreaCornerHelper
  {
    id: topLeft
    anchors.top: parent.top
    anchors.left: parent.left
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeFDiagCursor
    deltaXMultiplier: 1
    deltaYMultiplier: 1
    deltaWidthMultiplier: -1
    deltaHeightMultiplier: -1
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      if(root.expandable)
      {
        root.expandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      }
    }
  }

  MouseAreaCornerHelper
  {
    id: topRight
    anchors.top: parent.top
    anchors.right: parent.right
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeBDiagCursor
    deltaXMultiplier: 0
    deltaYMultiplier: 1
    deltaWidthMultiplier: 1
    deltaHeightMultiplier: -1
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      if(root.expandable)
      {
        root.expandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      }
    }
  }

  MouseAreaCornerHelper
  {
    id: bottomLeft
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeBDiagCursor
    deltaXMultiplier: 1
    deltaYMultiplier: 0
    deltaWidthMultiplier: -1
    deltaHeightMultiplier: 1
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      if(root.expandable)
      {
        root.expandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      }
    }
  }

  MouseAreaCornerHelper
  {
    id: bottomRight
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeFDiagCursor
    deltaXMultiplier: 0
    deltaYMultiplier: 0
    deltaWidthMultiplier: 1
    deltaHeightMultiplier: 1
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      if(root.expandable)
      {
        root.expandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      }
    }
  }
}
