import QtQuick
import QtQuick.Controls
import BibQml

Rectangle {
    id: testRect
    width: 400
    height: 300
    color: "#f0f0f0"

    Column {
        anchors.centerIn: testRect
        spacing: 20

        Label {
            text: "SettingBinder Singleton Test"
            font.pixelSize: 20
            font.bold: true
        }

        Button {
            text: "Test Singleton"
            onClicked: {
                console.log("Testing SettingBinder singleton...")
                var result = SettingBinder.test()
                console.log("Result: " + result)
                resultLabel.text = result
            }
        }

        Label {
            id: resultLabel
            text: "Click button to test"
            font.pixelSize: 16
            color: "#333333"
        }

        Label {
            text: "Singleton available: " + (SettingBinder !== undefined ? "Yes" : "No")
            font.pixelSize: 14
            color: SettingBinder !== undefined ? "green" : "red"
        }
    }

    Component.onCompleted: {
        console.log("TestSettingBinder loaded")
        console.log("SettingBinder object:", SettingBinder)
    }
}
