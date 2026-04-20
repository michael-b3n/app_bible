pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import BibQml

Item
{
  id: root

  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property AbstractListModelPassage listModelPassage

  required property int stepSize
  required property int margin

  readonly property bool runningState : bridgeBibleRefOcr.running

  implicitWidth: 640
  implicitHeight: 480

  signal closeClicked()

  ColumnLayout
  {
    anchors.fill: parent
    anchors.margins: root.margin
    spacing: root.margin

    RowLayout
    {
      // Object properties
      Layout.fillWidth: true
      Layout.fillHeight: false
      Layout.preferredHeight: root.stepSize
      spacing: root.margin

      TabBar
      {
        id: bar

        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
        background: Rectangle { color: "transparent" }

        MainTabButton
        {
          id: tab1
          anchors.top: parent.top
          anchors.left: parent.left
          width: bar.height
          height: bar.height
          runningState: root.runningState
        }

        SettingsTabButton
        {
          id: tab2
          anchors.top: parent.top
          anchors.left: tab1.right
          anchors.leftMargin: root.margin
          width: bar.height
          height: bar.height
        }
      }

      ButtonIconSimple
      {
        id: closeButton

        Layout.fillHeight: true
        Layout.fillWidth: false
        Layout.preferredWidth: root.stepSize
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

      MainTabContent
      {
        bridgeBibleRefOcr: root.bridgeBibleRefOcr
        listModelPassage: root.listModelPassage
      }

      SettingsTabContent
      {
      }
    }
  }
}
