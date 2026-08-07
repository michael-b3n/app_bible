import QtQuick

///
/// Mouse area that lets the call site move and resize what it covers. Dragging the center asks
/// for a move, dragging one of the four corners asks for a resize. The area only reports the
/// deltas of the drag, where its target ends up is up to the call site.
///
Item
{
  id: root

  // Properties
  required property bool expandable
  required property int expandAreaWidth
  required property bool movable

  readonly property bool containsMouse:
    center.containsMouse ||
    topLeft.containsMouse ||
    topRight.containsMouse ||
    bottomLeft.containsMouse ||
    bottomRight.containsMouse

  // Signals
  signal pressed(mouse: MouseEvent)
  signal released(mouse: MouseEvent)
  signal expandRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)
  signal moveRequested(deltaX: int, deltaY: int)

  // Components
  ///
  /// Center of the area, it moves the target.
  ///
  MouseArea
  {
    id: center

    // Properties
    // Position the drag started at
    property int clickX: 0
    property int clickY: 0

    anchors.fill: parent
    hoverEnabled: true

    // Connections
    onPressed: (mouse) =>
    {
      center.clickX = mouse.x
      center.clickY = mouse.y
      root.pressed(mouse)
    }
    onReleased: (mouse) => { root.released(mouse) }
    onPositionChanged: (mouse) =>
    {
      if(root.movable && center.pressed)
      {
        root.moveRequested(mouse.x - center.clickX, mouse.y - center.clickY)
      }
    }
  }

  ///
  /// Corners of the area, they resize the target.
  ///
  MoveResizeCorner
  {
    id: topLeft

    // Properties
    anchors.top: parent.top
    anchors.left: parent.left
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeFDiagCursor
    deltaXMultiplier: 1
    deltaYMultiplier: 1
    deltaWidthMultiplier: -1
    deltaHeightMultiplier: -1

    // Connections
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  MoveResizeCorner
  {
    id: topRight

    // Properties
    anchors.top: parent.top
    anchors.right: parent.right
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeBDiagCursor
    deltaXMultiplier: 0
    deltaYMultiplier: 1
    deltaWidthMultiplier: 1
    deltaHeightMultiplier: -1

    // Connections
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  MoveResizeCorner
  {
    id: bottomLeft

    // Properties
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeBDiagCursor
    deltaXMultiplier: 1
    deltaYMultiplier: 0
    deltaWidthMultiplier: -1
    deltaHeightMultiplier: 1

    // Connections
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  MoveResizeCorner
  {
    id: bottomRight

    // Properties
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeFDiagCursor
    deltaXMultiplier: 0
    deltaYMultiplier: 0
    deltaWidthMultiplier: 1
    deltaHeightMultiplier: 1

    // Connections
    onPressed: (mouse) => { root.pressed(mouse) }
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  // Functions
  ///
  /// Reports the resize a corner was dragged by, if this area may be resized at all.
  ///
  function requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
  {
    if(root.expandable)
    {
      root.expandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }
}
