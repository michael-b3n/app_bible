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
  // The window follows the phase it is told directly. Deriving its visibility from the fade of
  // its content instead would tie the window to an animation that only runs while it is on the
  // screen, and every restart of that fade would take the window off the screen again.
  visible: root.shown
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
    opacity: 0
    // The content holds the keyboard of the window, which is what lets it answer the escape key
    // of whatever the user is on.
    focus: true

    // Connections
    ///
    /// Escape hides the window, like its close button does. It is answered here and not by a
    /// shortcut of the application, so that what is open inside the window, e.g. the popup of a
    /// combo box, is closed by it first and the window only once nothing is left to close.
    ///
    Keys.onEscapePressed: (event) =>
    {
      root.closeClicked()
      event.accepted = true
    }

    // Animations
    // The window appears animated and disappears at once: what it shows belongs to the search
    // that asked for it, so it must not linger over what the user turns to next.
    states: State
    {
      name: "shown"
      when: root.shown

      PropertyChanges { content.opacity: 1 }
    }
    transitions: Transition
    {
      to: "shown"

      NumberAnimation
      {
        property: "opacity"
        duration: Metrics.durationShort
        easing.type: Easing.InOutQuad
      }
    }

    // Components
    MoveResizeArea
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
