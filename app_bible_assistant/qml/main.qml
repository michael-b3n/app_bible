import QtQuick
import QtQuick.Controls.Universal
import QtQuick.Layouts
import QtQuick.VectorImage
import BibQml

Window
{
  Universal.theme: Universal.Dark
  Universal.accent: Universal.Violet

  id: root

  required property BridgeBibleRefOcr bridge

  // Constants
  /*no binding*/ readonly property double goldenRatio: 1.618
  /*no binding*/ readonly property int stepSize: 32
  /*no binding*/ readonly property int margin: 8

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
  /*no binding*/ property int offsetToCursorX: -stepSize * goldenRatio
  /*no binding*/ property int offsetToCursorY: -40
  /*no binding*/ property int actualWidth: actualHeight * goldenRatio * 2
  /*no binding*/ property int actualHeight: stepSize

  // Bubble bounds
  readonly property int bubbleLeft: root.cursorX + root.offsetToCursorX
  readonly property int bubbleTop: root.cursorY + root.offsetToCursorY
  readonly property int bubbleRight: bubbleLeft + root.actualWidth
  readonly property int bubbleBottom: bubbleTop + root.actualHeight

  // Window content bounds (must contain both bubble and cursor)
  readonly property int contentLeft: Math.min(bubbleLeft, root.cursorX)
  readonly property int contentTop: Math.min(bubbleTop, root.cursorY)
  readonly property int contentRight: Math.max(bubbleRight, root.cursorX + 1)
  readonly property int contentBottom: Math.max(bubbleBottom, root.cursorY + 1)

  // Window position and size (constrained to screen)
  readonly property int windowX: Math.max(root.screenLeftBorder, Math.min(contentLeft, root.screenRightBorder - (contentRight - contentLeft)))
  readonly property int windowY: Math.max(root.screenTopBorder, Math.min(contentTop, root.screenBottomBorder - (contentBottom - contentTop)))
  readonly property int windowWidth: Math.min(root.screenGeometry.width, contentRight - contentLeft)
  readonly property int windowHeight: Math.min(root.screenGeometry.height, contentBottom - contentTop)

  // Object properties
  visible: speechBubble.opacity > 0
  color: "transparent"
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
  x: windowX
  y: windowY
  width: windowWidth
  height: windowHeight

  // Connections
  onVisibleChanged:
  {
    if(visible)
    {
      root.raise()
      root.requestActivate()
    }
  }

  // Helper functions
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
    newWidth = Math.max(newWidth, root.stepSize * root.goldenRatio * 2)
    newHeight = Math.max(newHeight, root.stepSize)
    return {x: newOffsetX, y: newOffsetY, width: newWidth, height: newHeight}
  }

  Connections
  {
    target: root.bridge

    function onCursorPositionChanged(cursorPosition)
    {
      root.cursorX = cursorPosition.x
      root.cursorY = cursorPosition.y
      root.screenGeometry = ScreenGeometryHelper.screenGeometryAt(cursorPosition)
      let constrained = root.constrainOffset(
          root.offsetToCursorX,
          root.offsetToCursorY,
          root.actualWidth,
          root.actualHeight
        )
      /*no binding*/ root.offsetToCursorX = constrained.x
      /*no binding*/ root.offsetToCursorY = constrained.y
    }

    function onVisibleChanged(visible)
    {
      speechBubble.opacity = visible || mouseAreaHelper.containsMouse ? 1 : 0
      loadingIcon.visible = visible
    }
  }

  // Children
  ShapeSpeechBubble
  {
    id: speechBubble

    opacity: 0
    anchors.fill: parent
    tailPositionX: root.cursorX - root.x
    tailPositionY: root.cursorY - root.y
    offsetToTailX: root.offsetToCursorX
    offsetToTailY: root.offsetToCursorY
    bubbleWidth: root.actualWidth
    bubbleHeight: root.actualHeight

    Behavior on opacity
    {
      NumberAnimation
      {
        duration: 400
        easing.type: Easing.InOutQuad
      }
    }

    Connections
    {
      target: mouseAreaHelper

      function onPressed(mouse)
      {
        root.x = root.screenGeometry.x
        root.y = root.screenGeometry.y
        root.width = root.screenGeometry.width
        root.height = root.screenGeometry.height
      }

      function onReleased(mouse)
      {
        root.x = root.windowX
        root.y = root.windowY
        root.width = root.windowWidth
        root.height = root.windowHeight
      }

      function onMoveRequested(deltaX, deltaY)
      {
        let constrained = root.constrainOffset(
          root.offsetToCursorX + deltaX,
          root.offsetToCursorY + deltaY,
          root.actualWidth,
          root.actualHeight
        )
        /*no binding*/ root.offsetToCursorX = constrained.x
        /*no binding*/ root.offsetToCursorY = constrained.y
      }

      function onExpandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      {
        let constrained = root.constrainSize(
          root.offsetToCursorX + deltaX,
          root.offsetToCursorY + deltaY,
          root.actualWidth + deltaWidth,
          root.actualHeight + deltaHeight
        )
        /*no binding*/ root.offsetToCursorX = constrained.x
        /*no binding*/ root.offsetToCursorY = constrained.y
        /*no binding*/ root.actualWidth = constrained.width
        /*no binding*/ root.actualHeight = constrained.height
      }
    }

    MouseAreaHelper // Mouse area for interaction
    {
      id: mouseAreaHelper

      property bool ignoreMouse: false

      expandable: true
      expandAreaWidth: root.margin
      movable: true
      x: speechBubble.bubbleX
      y: speechBubble.bubbleY
      width: speechBubble.bubbleWidth
      height: speechBubble.bubbleHeight

      // Children
      RowLayout
      {
        // Object properties
        x: speechBubble.radius / 2
        y: speechBubble.radius / 2
        width: parent.width - speechBubble.radius
        height: parent.height - speechBubble.radius

        // Children
        VectorImage
        {
          id: loadingIcon
          Layout.preferredWidth: parent.height
          Layout.preferredHeight: parent.height
          Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

          source: "qrc:/qt/qml/ui/qml/res/loading.svg"
          preferredRendererType: VectorImage.CurveRenderer

          RotationAnimator on rotation
          {
            running: loadingIcon.visible
            loops: Animation.Infinite
            from: 0;
            to: 360;
            duration: 3000
          }
        }

        VectorImage
        {
          id: foundIcon
          visible: !loadingIcon.visible
          Layout.preferredWidth: parent.height
          Layout.preferredHeight: parent.height
          Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

          source: "qrc:/qt/qml/ui/qml/res/check_mark.svg"
          preferredRendererType: VectorImage.CurveRenderer
        }

        ButtonIconSimple
        {
          Layout.preferredWidth: parent.height
          Layout.preferredHeight: parent.height
          Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
          svgSource: "qrc:/qt/qml/ui/qml/res/close.svg"

          onClicked:
          {
            speechBubble.opacity = 0
            root.bridge.visible = false
          }
        }
      }
    }
  }
}
