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

  readonly property bool runningState : bridgeBibleRefOcr.running

  implicitWidth: 640
  implicitHeight: 480

  // Signals
  signal closeClicked()

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
      }

      TabSettingsContent
      {
        listModelSettings: root.listModelSettings
      }
    }
  }
}
