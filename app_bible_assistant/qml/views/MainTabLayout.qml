import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import BibQml

///
/// Content of the main window. All functionality is reachable through the tabs, the buttons
/// beside them belong to the window itself and are only reported to its owner.
///
Item
{
  id: root

  // Properties
  required property SettingsListModel listModelSettings
  required property ScriptureListModel listModelScripture
  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property BridgeBibleRefLookup bridgeBibleRefLookup
  required property bool pinned
  required property bool movable

  // Constants
  // Index of the tab a found reference is shown in
  readonly property int scriptureTabIndex: 0

  implicitWidth: 640
  implicitHeight: 480

  // Signals
  signal closeClicked()
  signal pinClicked()
  signal released()
  signal moveRequested(deltaX: int, deltaY: int)

  // Connections
  ///
  /// Switches to the scripture tab, a reference found while another tab is open would stay
  /// hidden otherwise.
  ///
  Connections
  {
    target: root.bridgeBibleRefOcr

    function onReferenceFound(bookId, chapter, verse)
    {
      bar.setCurrentIndex(root.scriptureTabIndex)
    }
  }

  // Components
  ColumnLayout
  {
    // Properties
    anchors.fill: parent
    anchors.margins: Metrics.spacingSmall
    spacing: Metrics.spacingSmall

    // Components
    ///
    /// Header: the tabs and the buttons of the window.
    ///
    RowLayout
    {
      // Properties
      Layout.fillWidth: true
      Layout.fillHeight: false
      Layout.preferredHeight: Metrics.controlHeight
      spacing: Metrics.spacingSmall

      // Components
      TabBar
      {
        id: bar

        // Properties
        Layout.fillHeight: true
        Layout.fillWidth: false
        // Only as wide as the tabs it lays out, so that everything it does not need is left to
        // the move area beside it
        Layout.preferredWidth: bar.implicitWidth
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        spacing: Metrics.spacingSmall

        // Style
        background: Rectangle { color: "transparent" }

        // Components
        TabScriptureButton
        {
          // Properties
          searchRunning: root.bridgeBibleRefOcr.running
        }

        TabSettingsButton {}
      }

      ///
      /// Free space of the header beside the tabs, the one place the window is moved by.
      /// Dragging anywhere else would take it along while the user works in it.
      ///
      MoveArea
      {
        // Properties
        Layout.fillHeight: true
        Layout.fillWidth: true
        movable: root.movable

        // Connections
        onReleased: { root.released() }
        onMoveRequested: (deltaX, deltaY) => { root.moveRequested(deltaX, deltaY) }
      }

      ///
      /// Pins the window at its current position, or releases it back to the cursor.
      ///
      ButtonIconSwitch
      {
        // Properties
        Layout.fillHeight: true
        Layout.fillWidth: false
        Layout.preferredWidth: Metrics.controlHeight
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        svgSourceFirst: Icons.pin
        svgSourceSecond: Icons.pinFilled
        toggled: root.pinned

        // Connections
        onClicked: { root.pinClicked() }
      }

      ///
      /// Takes the window off the screen.
      ///
      ButtonIconSimple
      {
        // Properties
        Layout.fillHeight: true
        Layout.fillWidth: false
        Layout.preferredWidth: Metrics.controlHeight
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        svgSource: Icons.close

        // Connections
        onClicked: { root.closeClicked() }
      }
    }

    ///
    /// Content of the tab the user selected.
    ///
    StackLayout
    {
      // Properties
      Layout.fillWidth: true
      Layout.fillHeight: true
      currentIndex: bar.currentIndex

      // Components
      TabScriptureContent
      {
        // Properties
        listModelScripture: root.listModelScripture
        bridgeBibleRefLookup: root.bridgeBibleRefLookup
      }

      TabSettingsContent
      {
        // Properties
        listModelSettings: root.listModelSettings
      }
    }
  }
}
