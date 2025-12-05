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
  /*no binding*/ property var screenGeometry: ScreenGeometryHelper.screenGeometryAt(root.bridge.cursorPosition)
  /*no binding*/ readonly property int screenLeftBorder: screenGeometry.x
  /*no binding*/ readonly property int screenRightBorder: screenGeometry.x + screenGeometry.width
  /*no binding*/ readonly property int screenTopBorder: screenGeometry.y
  /*no binding*/ readonly property int screenBottomBorder: screenGeometry.y + screenGeometry.height

  // Cursor position
  /*no binding*/ readonly property int cursorX: root.bridge.cursorPosition.x
  /*no binding*/ readonly property int cursorY: root.bridge.cursorPosition.y

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

  // Window position and size
  readonly property int windowX: Math.max(root.screenLeftBorder, contentLeft)
  readonly property int windowY: Math.max(root.screenTopBorder, contentTop)
  readonly property int windowWidth: Math.min(root.screenGeometry.width, contentRight - windowX)
  readonly property int windowHeight: Math.min(root.screenGeometry.height, contentBottom - windowY)

  // Object properties
  visible: bridge.visible || speechBubble.opacity > 0
  color: "transparent"
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
  x: mouseAreaHelper.pressed ? screenLeftBorder : windowX
  y: mouseAreaHelper.pressed ? screenTopBorder : windowY
  width: mouseAreaHelper.pressed ? screenGeometry.width : windowWidth
  height: mouseAreaHelper.pressed ? screenGeometry.height : windowHeight

  // Connections
  onVisibleChanged:
  {
    if(visible)
    {
      mouseAreaHelper.ignoreMouse = false
      root.raise()
      root.requestActivate()
    }
  }

  Connections
  {
    target: root.bridge
    function onCursorPositionChanged(cursorPosition) { root.screenGeometry = ScreenGeometryHelper.screenGeometryAt(cursorPosition) }
  }

  // Children
  ShapeSpeechBubble
  {
    id: speechBubble

    opacity: root.bridge.visible || (!mouseAreaHelper.ignoreMouse && mouseAreaHelper.containsMouse) ? 1 : 0

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
        duration: 2000
        easing.type: Easing.InOutQuad
      }
    }

    Connections
    {
      target: mouseAreaHelper
      function onMoveRequested(deltaX, deltaY)
      {
        let newOffsetX = root.offsetToCursorX + deltaX
        let newOffsetY = root.offsetToCursorY + deltaY
        let newBubbleLeft = root.cursorX + newOffsetX
        let newBubbleTop = root.cursorY + newOffsetY
        let newBubbleRight = newBubbleLeft + root.actualWidth
        let newBubbleBottom = newBubbleTop + root.actualHeight

        // Constrain to screen bounds
        if(newBubbleLeft < root.screenLeftBorder)
        {
          newOffsetX = root.screenLeftBorder - root.cursorX
        }
        else if(newBubbleRight > root.screenRightBorder)
        {
          newOffsetX = root.screenRightBorder - root.actualWidth - root.cursorX
        }

        if(newBubbleTop < root.screenTopBorder)
        {
          newOffsetY = root.screenTopBorder - root.cursorY
        }
        else if(newBubbleBottom > root.screenBottomBorder)
        {
          newOffsetY = root.screenBottomBorder - root.actualHeight - root.cursorY
        }

        /*no binding*/ root.offsetToCursorX = newOffsetX
        /*no binding*/ root.offsetToCursorY = newOffsetY
      }

      function onExpandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      {
        let newOffsetX = root.offsetToCursorX + deltaX
        let newOffsetY = root.offsetToCursorY + deltaY
        let newWidth = root.actualWidth + deltaWidth
        let newHeight = root.actualHeight + deltaHeight
        let newBubbleLeft = root.cursorX + newOffsetX
        let newBubbleTop = root.cursorY + newOffsetY
        let newBubbleRight = newBubbleLeft + newWidth
        let newBubbleBottom = newBubbleTop + newHeight

        // Constrain to screen bounds
        if(newBubbleLeft < root.screenLeftBorder)
        {
          newWidth = newWidth - (root.screenLeftBorder - newBubbleLeft)
          newOffsetX = root.screenLeftBorder - root.cursorX
        }
        if(newBubbleTop < root.screenTopBorder)
        {
          newHeight = newHeight - (root.screenTopBorder - newBubbleTop)
          newOffsetY = root.screenTopBorder - root.cursorY
        }
        if(newBubbleRight > root.screenRightBorder)
        {
          newWidth = root.screenRightBorder - (root.cursorX + newOffsetX)
        }
        if(newBubbleBottom > root.screenBottomBorder)
        {
          newHeight = root.screenBottomBorder - (root.cursorY + newOffsetY)
        }

        // Enforce minimum size
        if(newWidth < root.stepSize * root.goldenRatio * 2)
        {
          newWidth = root.stepSize * root.goldenRatio * 2
        }
        if(newHeight < root.stepSize)
        {
          newHeight = root.stepSize
        }

        /*no binding*/ root.offsetToCursorX = newOffsetX
        /*no binding*/ root.offsetToCursorY = newOffsetY
        /*no binding*/ root.actualWidth = newWidth
        /*no binding*/ root.actualHeight = newHeight
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
          Layout.preferredWidth: parent.height
          Layout.preferredHeight: parent.height
          Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

          source: "qrc:/qt/qml/ui/qml/res/loading.svg"
          preferredRendererType: VectorImage.CurveRenderer

          RotationAnimator on rotation
          {
            running: root.visible
            loops: Animation.Infinite
            from: 0;
            to: 360;
            duration: 3000
          }
        }

        ButtonIconSimple
        {
          Layout.preferredWidth: parent.height
          Layout.preferredHeight: parent.height
          Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
          svgSource: "qrc:/qt/qml/ui/qml/res/close.svg"

          onClicked:
          {
            mouseAreaHelper.ignoreMouse = true
            root.bridge.visible = false
          }
        }
      }
    }
  }
}
