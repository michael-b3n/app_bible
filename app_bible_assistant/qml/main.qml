import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import QtQuick.Layouts
import QtQuick.VectorImage
import BibQml

ApplicationWindow {
    Universal.theme: Universal.Dark
    Universal.accent: Universal.Violet

    required property BridgeBibleRefOcr bridge

    id: root
    visible: bridge.visible
    color: "transparent"
    width: 48
    height: 16
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    x: bridge.cursorX + root.width > Screen.desktopAvailableWidth ? bridge.cursorX - root.width : bridge.cursorX
    y: bridge.cursorY + root.height > Screen.desktopAvailableHeight ? bridge.cursorY - root.height : bridge.cursorY

    onVisibleChanged: {
        if (visible)
        {
            root.raise()
            root.requestActivate()
        }
    }

    Rectangle {
        id: mainRect
        anchors.fill: parent
        color: Universal.background
        radius: root.height * 0.1

        RowLayout {
            id: layout
            anchors.fill: parent
            spacing: root.height * 0.05

            Button {
                id: closeButton
                Layout.preferredWidth: parent.height
                Layout.preferredHeight: parent.height

                VectorImage {
                    width: parent.width
                    height: parent.height
                    preferredRendererType: VectorImage.CurveRenderer
                    source: "qrc:/qt/qml/ui/qml/res/close.svg"
                }

                onClicked: bridge.visible = false
            }

            ProgressBar {
                id: control
                Layout.fillWidth: true
                Layout.fillHeight: true
                indeterminate: true
            }
        }
    }
}
