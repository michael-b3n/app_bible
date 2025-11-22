pragma Singleton
import QtQuick

QtObject {
    // Notification Widget Colors
    readonly property color notificationBackground: "#2D2D30"
    readonly property color notificationBorder: "#007ACC"
    readonly property color notificationText: "#FFFFFF"
    readonly property color notificationCloseButton: "#C75050"
    readonly property color notificationCloseButtonHover: "#E81123"

    // Progress Bar Colors
    readonly property color progressBarBackground: "#3E3E42"
    readonly property color progressBarForeground: "#007ACC"

    // Dimensions
    readonly property int notificationWidth: 300
    readonly property int notificationHeight: 80
    readonly property int notificationBorderWidth: 2
    readonly property int notificationRadius: 4
    readonly property int notificationPadding: 12

    // Typography
    readonly property int fontSize: 12
    readonly property string fontFamily: "Segoe UI"

    // Animation
    readonly property int animationDuration: 200

    // Close Button
    readonly property int closeButtonSize: 20
    readonly property int closeButtonMargin: 4
}
