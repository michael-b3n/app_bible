import QtQuick
import BibQml

///
/// Window carrying the content. It has no decoration of its own, it is moved and resized by
/// its mouse area and framed by the speech bubble behind it.
///
Window
{
  id: root

  // Properties
  required property SettingsListModel listModelSettings
  required property ScriptureListModel listModelScripture
  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property BridgeBibleRefLookup bridgeBibleRefLookup
  required property rect mainRect
  required property bool pinned
  required property bool shown

  color: "transparent"
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
  // The fade runs on the content: a hidden window reports no opacity change, so a visibility
  // bound to its own opacity would never become true.
  visible: content.opacity > 0
  x: root.mainRect.x
  y: root.mainRect.y
  width: root.mainRect.width
  height: root.mainRect.height

  // Signals
  signal released()
  signal moveRequested(deltaX: int, deltaY: int)
  signal expandRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)
  signal closeClicked()
  signal pinClicked()

  // Components
  Item
  {
    id: content

    // Properties
    anchors.fill: parent
    opacity: root.shown ? 1 : 0

    // Animations
    Behavior on opacity
    {
      NumberAnimation
      {
        duration: Metrics.durationShort
        easing.type: Easing.InOutQuad
      }
    }

    // Components
    MouseAreaHelper
    {
      // Properties
      anchors.fill: parent
      expandable: true
      expandAreaWidth: Metrics.spacingMedium
      movable: true

      // Connections
      onReleased: { root.released() }
      onMoveRequested: (deltaX, deltaY) => { root.moveRequested(deltaX, deltaY) }
      onExpandRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
      {
        root.expandRequested(deltaX, deltaY, deltaWidth, deltaHeight)
      }

      // Components
      MainTabLayout
      {
        // Properties
        listModelSettings: root.listModelSettings
        listModelScripture: root.listModelScripture
        bridgeBibleRefOcr: root.bridgeBibleRefOcr
        bridgeBibleRefLookup: root.bridgeBibleRefLookup
        pinned: root.pinned

        anchors.fill: parent

        // Connections
        onCloseClicked: { root.closeClicked() }
        onPinClicked: { root.pinClicked() }
      }
    }
  }
}
