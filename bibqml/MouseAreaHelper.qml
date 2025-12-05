import QtQuick

Item
{
  id: root

  required property bool expandable
  required property int expandAreaWidth
  required property bool movable

  readonly property bool containsMouse:
    mouseAreaCenter.containsMouse ||
    mouseAreaTopLeft.containsMouse || mouseAreaTopLeft.pressed ||
    mouseAreaTopRight.containsMouse || mouseAreaTopRight.pressed ||
    mouseAreaBottomLeft.containsMouse || mouseAreaBottomLeft.pressed ||
    mouseAreaBottomRight.containsMouse || mouseAreaBottomRight.pressed

  readonly property bool pressed:
    mouseAreaCenter.pressed ||
    mouseAreaTopLeft.pressed ||
    mouseAreaTopRight.pressed ||
    mouseAreaBottomLeft.pressed ||
    mouseAreaBottomRight.pressed

  implicitHeight: 100
  implicitWidth: 200

  signal expandRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)
  signal moveRequested(deltaX: int, deltaY: int)

  MouseArea
  {
    id: mouseAreaCenter

    property int clickX: 0
    property int clickY: 0

    hoverEnabled: true
    anchors.fill: parent
    cursorShape: Qt.SizeAllCursor

    onPressed: (mouse) =>
    {
      clickX = mouse.x
      clickY = mouse.y
    }
    onPositionChanged: (mouse) =>
    {
      if(root.movable && pressed)
      {
        root.moveRequested(mouse.x - clickX, mouse.y - clickY)
      }
    }
  }

  MouseArea
  {
    id: mouseAreaTopLeft

    property int clickX: 0
    property int clickY: 0

    hoverEnabled: true
    anchors.top: parent.top
    anchors.left: parent.left
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeFDiagCursor

    onPressed: (mouse) =>
    {
      clickX = mouse.x
      clickY = mouse.y
    }
    onPositionChanged: (mouse) =>
    {
      if(root.expandable && pressed)
      {
        let deltaX = mouse.x - clickX
        let deltaY = mouse.y - clickY
        root.expandRequested(deltaX, deltaY, -deltaX, -deltaY)
      }
    }
  }

  MouseArea
  {
    id: mouseAreaTopRight

    property int clickX: 0
    property int clickY: 0

    hoverEnabled: true
    anchors.top: parent.top
    anchors.right: parent.right
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeBDiagCursor

    onPressed: (mouse) =>
    {
      clickX = mouse.x
      clickY = mouse.y
    }
    onPositionChanged: (mouse) =>
    {
      if(root.expandable && pressed)
      {
        let deltaX = mouse.x - clickX
        let deltaY = mouse.y - clickY
        root.expandRequested(0, deltaY, deltaX, -deltaY)
      }
    }
  }

  MouseArea
  {
    id: mouseAreaBottomLeft

    property int clickX: 0
    property int clickY: 0

    hoverEnabled: true
    anchors.bottom: parent.bottom
    anchors.left: parent.left
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeBDiagCursor

    onPressed: (mouse) =>
    {
      clickX = mouse.x
      clickY = mouse.y
    }
    onPositionChanged: (mouse) =>
    {
      if(root.expandable && pressed)
      {
        let deltaX = mouse.x - clickX
        let deltaY = mouse.y - clickY
        root.expandRequested(deltaX, 0, -deltaX, deltaY)
      }
    }
  }

  MouseArea
  {
    id: mouseAreaBottomRight

    property int clickX: 0
    property int clickY: 0

    hoverEnabled: true
    anchors.bottom: parent.bottom
    anchors.right: parent.right
    width: root.expandAreaWidth
    height: root.expandAreaWidth
    cursorShape: Qt.SizeFDiagCursor

    onPressed: (mouse) =>
    {
      clickX = mouse.x
      clickY = mouse.y
    }
    onPositionChanged: (mouse) =>
    {
      if(root.expandable && pressed)
      {
        let deltaX = mouse.x - clickX
        let deltaY = mouse.y - clickY
        root.expandRequested(0, 0, deltaX, deltaY)
      }
    }
  }
}
