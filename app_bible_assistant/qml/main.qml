import QtQuick
import QtQuick.Controls
import BibQml

ApplicationWindow {
    id: root

    visible: true
    width: 640
    height: 480
    title: qsTr("Bible Assistant - BibQml Test")

    TestSettingBinder {
        anchors.fill: root.contentItem
    }
}
