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

  // Constants
  // Index of the tab a found reference is shown in
  readonly property int scriptureTabIndex: 0

  implicitWidth: 640
  implicitHeight: 480

  // Signals
  signal closeClicked()
  signal pinClicked()

  // Connections
  ///
  /// A found reference is shown in the scripture tab, so switch to it. Otherwise the
  /// scripture of a reference found while another tab is open would stay hidden.
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
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

        // Style
        background: Rectangle { color: "transparent" }

        // Components
        TabScriptureButton
        {
          id: scriptureTab

          // Properties
          anchors.top: parent.top
          anchors.left: parent.left
          width: bar.height
          height: bar.height
          searchRunning: root.bridgeBibleRefOcr.running
        }

        TabSettingsButton
        {
          // Properties
          anchors.top: parent.top
          anchors.left: scriptureTab.right
          anchors.leftMargin: Metrics.spacingSmall
          width: bar.height
          height: bar.height
        }
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
