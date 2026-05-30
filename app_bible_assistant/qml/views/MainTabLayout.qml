import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import BibQml

Item
{
  id: root

  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property ScriptureListModel listModelPassage

  readonly property bool runningState : bridgeBibleRefOcr.running

  implicitWidth: 640
  implicitHeight: 480

  signal closeClicked()

  ColumnLayout
  {
    anchors.fill: parent
    anchors.margins: Metrics.spacingSmall
    spacing: Metrics.spacingSmall

    RowLayout
    {
      // Object properties
      Layout.fillWidth: true
      Layout.fillHeight: false
      Layout.preferredHeight: Metrics.controlHeight
      spacing: Metrics.spacingSmall

      TabBar
      {
        id: bar

        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        background: Rectangle { color: "transparent" }

        TabScriptureButton
        {
          id: tab1
          anchors.top: parent.top
          anchors.left: parent.left
          width: bar.height
          height: bar.height
          runningState: root.runningState
        }

        TabSettingsButton
        {
          id: tab2
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

        Layout.fillHeight: true
        Layout.fillWidth: false
        Layout.preferredWidth: Metrics.controlHeight
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

        svgSource: "qrc:/qt/qml/ui/qml/res/close.svg"
        onClicked: { root.closeClicked() }
      }
    }

    StackLayout
    {
      Layout.fillWidth: true
      Layout.fillHeight: true
      currentIndex: bar.currentIndex

      TabScriptureContent
      {
        bridgeBibleRefOcr: root.bridgeBibleRefOcr
        listModelPassage: root.listModelPassage
      }

      TabSettingsContent
      {
      }
    }
  }
}
