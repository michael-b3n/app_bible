import QtQuick

///
/// Mouse area that lets the call site resize what it covers by dragging one of its four corners.
/// The area only reports the deltas of the drag, where its target ends up is up to the call site.
/// Everything but the corners is left to what the area covers, so that the content of a window
/// stays reachable, which is why this area does not move its target. That is the job of a
/// MoveArea placed where the window has nothing else to do.
///
Item
{
  id: root

  // Properties
  required property bool expandable
  required property int expandAreaWidth

  // Signals
  signal released(mouse: MouseEvent)
  signal expandRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)

  // Components
  ///
  /// Corners of the area, they resize the target.
  ///
  ExpandCorner
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
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  ExpandCorner
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
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  ExpandCorner
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
    onReleased: (mouse) => { root.released(mouse) }
    onResizeRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.requestExpand(deltaX, deltaY, deltaWidth, deltaHeight)
    }
  }

  ExpandCorner
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
