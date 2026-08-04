import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import BibQml

///
/// This object describes the tab layout.
/// All main functionality is accessible
/// via the provided tabs.
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

  readonly property bool runningState : bridgeBibleRefOcr.running

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
      bar.setCurrentIndex(0)
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
        background: Rectangle { color: "transparent" }

        // Components
        TabScriptureButton
        {
          id: tab1

          // Properties
          anchors.top: parent.top
          anchors.left: parent.left
          width: bar.height
          height: bar.height
          runningState: root.runningState
        }

        TabSettingsButton
        {
          id: tab2

          // Properties
          anchors.top: parent.top
          anchors.left: tab1.right
          anchors.leftMargin: Metrics.spacingSmall
          width: bar.height
          height: bar.height
        }
      }

      ButtonIconSwitch
      {
        id: pinButton

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

      ButtonIconSimple
      {
        id: closeButton

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

    StackLayout
    {
      // Properties
      Layout.fillWidth: true
      Layout.fillHeight: true
      currentIndex: bar.currentIndex

      // Components
      TabScriptureContent
      {
        listModelScripture: root.listModelScripture
        bridgeBibleRefOcr: root.bridgeBibleRefOcr
        bridgeBibleRefLookup: root.bridgeBibleRefLookup
      }

      TabSettingsContent
      {
        listModelSettings: root.listModelSettings
      }
    }
  }
}
