import QtQuick
import QtQuick.Controls.Universal
import BibQml


QtObject
{
  id: root

  required property BridgeBibleRefOcr bridgeBibleRefOcr

  // Constants
  /*no binding*/ readonly property double goldenRatio: 1.618
  /*no binding*/ readonly property int stepSize: 24
  /*no binding*/ readonly property int margin: 4
  /*no binding*/ readonly property int radius : margin * 2
  /*no binding*/ readonly property int opacityDuration: 200
  /*no binding*/ readonly property int tailLengthMax: stepSize * 2
  /*no binding*/ readonly property int minimalWidth: stepSize * goldenRatio * 3
  /*no binding*/ readonly property int minimalHeight: stepSize + 2 * margin

  // Screen geometry
  /*no binding*/ property var screenGeometry: ScreenGeometryHelper.screenGeometryAt({ x: 0, y: 0 })
  /*no binding*/ readonly property int screenLeftBorder: screenGeometry.x
  /*no binding*/ readonly property int screenRightBorder: screenGeometry.x + screenGeometry.width
  /*no binding*/ readonly property int screenTopBorder: screenGeometry.y
  /*no binding*/ readonly property int screenBottomBorder: screenGeometry.y + screenGeometry.height

  // Cursor position
  /*no binding*/ property int cursorX: 0
  /*no binding*/ property int cursorY: 0

  // Bubble offset and size
  /*no binding*/ property int userOffsetToCursorX: -stepSize * goldenRatio
  /*no binding*/ property int userOffsetToCursorY: -40
  /*no binding*/ property int offsetToCursorX: -stepSize * goldenRatio
  /*no binding*/ property int offsetToCursorY: -40
  /*no binding*/ property int mainWidth: minimalWidth
  /*no binding*/ property int mainHeight: minimalHeight

  Universal.theme: Universal.Dark
  Universal.accent: Universal.Violet

  // Connections
  property Connections bridgeConnections: Connections
  {
    target: root.bridgeBibleRefOcr

    function onCursorPositionChanged(cursorPosition)
    {
      root.cursorX = cursorPosition.x
      root.cursorY = cursorPosition.y
      root.screenGeometry = ScreenGeometryHelper.screenGeometryAt(cursorPosition)
      Qt.callLater(function()
      {
        let constrained = root.constrainOffset(
            root.userOffsetToCursorX,
            root.userOffsetToCursorY,
            root.mainWidth,
            root.mainHeight
          )
        /*no binding*/ root.offsetToCursorX = constrained.x
        /*no binding*/ root.offsetToCursorY = constrained.y
      })
    }

    function onRunningChanged(running)
    {
      Qt.callLater(function()
      {
        if(running)
        {
          background.raise()
          root.show()
        }
        else if(!mouseAreaHelper.containsMouse)
        {
          // root.hide()
        }
      })
    }
  }

  // Background window with speech bubble shape
  property Window background: Window
  {
    id: background

    // Object properties
    visible: speechBubble.opacity > 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    x: root.screenGeometry.x
    y: root.screenGeometry.y
    width: root.screenGeometry.width
    height: root.screenGeometry.height

    onVisibleChanged: (visible) => { if(visible) { Qt.callLater(function() { main.raise() }) } }

    // Children
    ShapeSpeechBubble
    {
      id: speechBubble

      anchors.fill: parent
      opacity: 0
      radius: root.radius
      tailPositionX: root.cursorX - background.x
      tailPositionY: root.cursorY - background.y
      offsetToTailX: root.offsetToCursorX
      offsetToTailY: root.offsetToCursorY
      bubbleWidth: root.mainWidth
      bubbleHeight: root.mainHeight

      Behavior on opacity
      {
        NumberAnimation
        {
          duration: root.opacityDuration
          easing.type: Easing.InOutQuad
        }
      }
    }
  }

  // Main window with simple rounded rectangle shape
  property Window main: Window
  {
    id: main

    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    opacity: Math.min(bubble.opacity + speechBubble.opacity, 1)
    visible: bubble.opacity > 0 || speechBubble.opacity > 0
    x: root.cursorX + root.offsetToCursorX
    y: root.cursorY + root.offsetToCursorY
    width: root.mainWidth
    height: root.mainHeight

    Rectangle
    {
      id: bubble

      anchors.fill: parent
      border.color: Colors.border
      color: Colors.backgroundTransparent
      opacity: 0
      radius: root.radius

      Behavior on opacity
      {
        NumberAnimation
        {
          duration: root.opacityDuration
          easing.type: Easing.InOutQuad
        }
      }
    }

    MouseAreaHelper // Mouse area for interaction
    {
      id: mouseAreaHelper

      anchors.fill: parent
      expandable: true
      expandAreaWidth: root.margin
      movable: true

      onReleased: (mouse) => { root.show() }
      onMoveRequested: (deltaX, deltaY) =>
      {
        root.userOffsetToCursorX = root.offsetToCursorX + deltaX
        root.userOffsetToCursorY = root.offsetToCursorY + deltaY
        let constrained = root.constrainOffset(
          root.userOffsetToCursorX,
          root.userOffsetToCursorY,
          root.mainWidth,
          root.mainHeight
        )
        /*no binding*/ root.offsetToCursorX = constrained.x
        /*no binding*/ root.offsetToCursorY = constrained.y
      }
      onExpandRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
      {
        root.userOffsetToCursorX = root.offsetToCursorX + deltaX
        root.userOffsetToCursorY = root.offsetToCursorY + deltaY
        let constrained = root.constrainSize(
          root.userOffsetToCursorX,
          root.userOffsetToCursorY,
          root.mainWidth + deltaWidth,
          root.mainHeight + deltaHeight
        )
        /*no binding*/ root.offsetToCursorX = constrained.x
        /*no binding*/ root.offsetToCursorY = constrained.y
        /*no binding*/ root.mainWidth = constrained.width
        /*no binding*/ root.mainHeight = constrained.height
      }

      MainTabLayout
      {
        id: mainTabLayout

        bridgeBibleRefOcr: root.bridgeBibleRefOcr

        stepSize: root.stepSize
        margin: root.margin
        anchors.fill: parent

        onCloseClicked: () =>
        {
          Qt.callLater(function()
          {
            root.hide()
          })
        }
      }
    }
  }

  // Helper functions
  //
  // Shows the background shape adjusted to content bounds.
  //
  function show()
  {
    if(showTail())
    {
      speechBubble.opacity = 1
      bubble.opacity = 0
    }
    else
    {
      bubble.opacity = 1
      speechBubble.opacity = 0
    }
    Qt.callLater(function() { main.raise() })
  }

  //
  // Hides both foreground and background shapes and windows.
  //
  function hide()
  {
    bubble.opacity = 0
    speechBubble.opacity = 0
  }

  //
  // Constrains the bubble offset to keep it within screen bounds
  // Returns adjusted offset coordinates that ensure the bubble stays fully visible
  //
  function constrainOffset(newOffsetX, newOffsetY, width, height)
  {
    let bubbleLeft = root.cursorX + newOffsetX
    let bubbleTop = root.cursorY + newOffsetY
    let bubbleRight = bubbleLeft + width
    let bubbleBottom = bubbleTop + height

    if(bubbleLeft < root.screenLeftBorder)
    {
      newOffsetX = root.screenLeftBorder - root.cursorX
    }
    else if(bubbleRight > root.screenRightBorder)
    {
      newOffsetX = root.screenRightBorder - width - root.cursorX
    }

    if(bubbleTop < root.screenTopBorder)
    {
      newOffsetY = root.screenTopBorder - root.cursorY
    }
    else if(bubbleBottom > root.screenBottomBorder)
    {
      newOffsetY = root.screenBottomBorder - height - root.cursorY
    }
    return {x: newOffsetX, y: newOffsetY}
  }

  //
  // Constrains the bubble size and offset during resize operations
  // Ensures the bubble stays within screen bounds and maintains minimum size
  // Returns adjusted offset and size that respect screen boundaries
  //
  function constrainSize(newOffsetX, newOffsetY, newWidth, newHeight)
  {
    let bubbleLeft = root.cursorX + newOffsetX
    let bubbleTop = root.cursorY + newOffsetY
    let bubbleRight = bubbleLeft + newWidth
    let bubbleBottom = bubbleTop + newHeight

    if(bubbleLeft < root.screenLeftBorder)
    {
      newWidth -= (root.screenLeftBorder - bubbleLeft)
      newOffsetX = root.screenLeftBorder - root.cursorX
    }
    if(bubbleTop < root.screenTopBorder)
    {
      newHeight -= (root.screenTopBorder - bubbleTop)
      newOffsetY = root.screenTopBorder - root.cursorY
    }
    if(bubbleRight > root.screenRightBorder)
    {
      newWidth = root.screenRightBorder - (root.cursorX + newOffsetX)
    }
    if(bubbleBottom > root.screenBottomBorder)
    {
      newHeight = root.screenBottomBorder - (root.cursorY + newOffsetY)
    }
    // Enforce minimum size
    newWidth = Math.max(newWidth, root.minimalWidth)
    newHeight = Math.max(newHeight, root.minimalHeight)
    return {x: newOffsetX, y: newOffsetY, width: newWidth, height: newHeight}
  }

  //
  // Check with the offsets and the bubble position if the tail of the speech bubble should be shown.
  //
  function showTail()
  {
    let leftX = root.cursorX + root.offsetToCursorX
    let rightX = leftX + root.mainWidth
    let topY = root.cursorY + root.offsetToCursorY
    let bottomY = topY + root.mainHeight
    let tailLengthMax = root.tailLengthMax

    let inRangeX = root.cursorX >= leftX - tailLengthMax && root.cursorX <= rightX + tailLengthMax
    let inRangeY = root.cursorY >= topY - tailLengthMax && root.cursorY <= bottomY + tailLengthMax
    return inRangeX && inRangeY
  }
}
