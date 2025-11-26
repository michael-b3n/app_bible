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

  readonly property int cursorX: bridge.cursorPosition.x
  readonly property int cursorY: bridge.cursorPosition.y
  property var screenGeometry: ScreenHelper.screenGeometryAt(bridge.cursorPosition)
  readonly property int screenLeft: screenGeometry.x
  readonly property int screenRight: screenGeometry.x + screenGeometry.width
  readonly property int screenTop: screenGeometry.y
  readonly property int screenBottom: screenGeometry.y + screenGeometry.height
  readonly property bool onLeftSide: cursorX - root.width / 2 < screenLeft
  readonly property bool onRightSide: cursorX + root.width / 2 > screenRight
  readonly property bool onTopSide: cursorY - root.height < screenTop
  readonly property int margin: height * 0.1
  property bool cursorInside: false

  // Object properties
  visible: bridge.visible || cursorInside
  color: "transparent"
  width: 120
  height: 40
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
  x: onRightSide ? screenRight - root.width : (onLeftSide ? screenLeft : cursorX - root.width / 2)
  y: onTopSide ? cursorY : cursorY - root.height

  // Connections
  onVisibleChanged:
  {
    if(visible)
    {
      root.raise()
      root.requestActivate()
    }
  }

  Connections
  {
    target: root.bridge
    function onCursorPositionChanged(cursorPosition) { root.screenGeometry = ScreenHelper.screenGeometryAt(cursorPosition) }
  }

  // Children
  ShapeSpeechBubble
  {
    id: speechBubble
    anchors.margins: root.margin
    anchors.fill: parent
    tailPositionX: root.cursorX - root.x - root.margin /*margin correction*/
    upwards: !root.onTopSide

    // We don't drag for now but we could expand and the expanded
    // window can be draggable.
    // MouseArea // drag mouse area
    // {
    //   anchors.fill: parent
    //   property int clickX: 0
    //   property int clickY: 0

    //   onPressed: (mouse) =>
    //   {
    //     clickX = mouse.x
    //     clickY = mouse.y
    //   }
    //   onPositionChanged: (mouse) =>
    //   {
    //     let delta = Qt.point(mouse.x - clickX, mouse.y - clickY)
    //     root.x += delta.x;
    //     root.y += delta.y;
    //   }
    // }

    MouseArea // Mouse area for cursor inside detection
    {
      // Object properties
      anchors.fill: parent
      hoverEnabled: true

      // Connections
      onEntered: { root.cursorInside = true }
      onExited: { root.cursorInside = false }

      // Children
      RowLayout
      {
        // Object properties
        x: speechBubble.innerX
        y: speechBubble.innerY
        width: speechBubble.innerWidth
        height: speechBubble.innerHeight

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
            root.cursorInside = false
            root.bridge.visible = false
          }
        }

        // ButtonIconSimple
        // {
        //   Layout.preferredWidth: parent.height
        //   Layout.preferredHeight: parent.height
        //   svgSource: "qrc:/qt/qml/ui/qml/res/pointer.svg"

        //   onClicked:
        //   {
        //     root.x = Qt.binding(function() { return root.xBinding })
        //     root.y = Qt.binding(function() { return root.yBinding })
        //   }
        // }
      }
    }
  }
}
