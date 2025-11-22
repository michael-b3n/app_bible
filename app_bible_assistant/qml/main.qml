import QtQuick
import QtQuick.Controls
import BibQml

ApplicationWindow {
    required property BridgeBibleRefOcr bridge

    id: root
    visible: bridge.visible
    width: 640
    height: 480
    title: qsTr("Bible Assistant")
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    x: bridge.cursorX
    y: bridge.cursorY

    onVisibleChanged: {
        if (visible)
        {
            root.raise()
            root.requestActivate()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "lightgray"
        border.color: "black"
        border.width: 1
        radius: 5

        Row {
            anchors.fill: parent
            anchors.margins: 5
            spacing: 5

            Label {
                text: qsTr("Processing OCR...")
                verticalAlignment: Text.AlignVCenter
                width: parent.width - closeButton.width - spacing
            }

            Button {
                id: closeButton
                width: 20
                height: 20
                text: "×"

                onClicked: bridge.visible = false
            }
        }
    }
}
