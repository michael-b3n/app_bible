import QtQuick
import QtQuick.Shapes

///
/// Transparent button covering the reference on the screen. While the search runs it is shown in
/// its default size at the cursor and reports the search by a segment travelling along its border,
/// once the reference is found it grows onto it. It times out because the content below it may
/// have scrolled away or changed in the meantime.
///
Window
{
  id: root

  // Properties
  required property rect overlayRect
  required property bool loading
  required property bool shown
  required property int timeout

  // Area the window covers, it follows the requested one animated
  property rect currentRect: Qt.rect(0, 0, 0, 0)

  color: "transparent"
  // The overlay must not take the focus away from the window the user is reading.
  flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus
  // The fade runs on the content: a hidden window reports no opacity change, so a visibility
  // bound to its own opacity would never become true.
  visible: content.opacity > 0
  x: root.currentRect.x
  y: root.currentRect.y
  width: root.currentRect.width
  height: root.currentRect.height

  // Signals
  signal clicked()
  signal timedOut()

  // Connections
  Component.onCompleted: { root.applyOverlayRect() }
  // The requested area and the state it belongs to arrive as separate bindings, so the button
  // waits until both have settled before it decides to grow or to jump.
  onOverlayRectChanged: { Qt.callLater(root.applyOverlayRect) }
  onLoadingChanged: { Qt.callLater(root.applyOverlayRect) }
  onVisibleChanged: { Qt.callLater(root.applyOverlayRect) }

  // Animations
  PropertyAnimation
  {
    id: growAnimation

    // Properties
    target: root
    property: "currentRect"
    duration: Metrics.durationMedium
    easing.type: Easing.InOutQuad
  }

  // Components
  Item
  {
    id: content

    // Properties
    anchors.fill: parent
    opacity: 0

    // Animations
    // The button appears animated and disappears at once, together with the window it belongs to.
    states: State
    {
      name: "shown"
      when: root.shown

      PropertyChanges { content.opacity: 1 }
    }
    transitions: Transition
    {
      to: "shown"

      NumberAnimation
      {
        property: "opacity"
        duration: Metrics.durationShort
        easing.type: Easing.InOutQuad
      }
    }

    // Components
    DashedBorder
    {
      id: dashedBorder

      // Properties
      borderWidth: content.width
      borderHeight: content.height
      radius: Metrics.radiusLarge
      strokeWidth: Metrics.borderThick
      segmentRatio: 0.25
    }

    Rectangle
    {
      // Properties
      // The reference below shall stay readable, so the area is only tinted while it is hovered.
      // Note that the opacity never drops to zero: fully transparent pixels of a translucent
      // window are not hit by mouse events, which would make the overlay unclickable.
      anchors.fill: parent
      color: Colors.selection
      opacity: mouseArea.containsMouse ? 0.3 : 0.02
      border.color: Colors.border
      border.width: mouseArea.containsMouse ? Metrics.border : 0
      radius: Metrics.radiusLarge

      // Animations
      Behavior on opacity
      {
        NumberAnimation
        {
          duration: Metrics.durationShort
          easing.type: Easing.InOutQuad
        }
      }
    }

    Shape
    {
      // Properties
      anchors.fill: parent
      opacity: root.loading ? 1 : 0

      // Animations
      Behavior on opacity
      {
        NumberAnimation
        {
          duration: Metrics.durationShort
          easing.type: Easing.InOutQuad
        }
      }

      // Components
      ShapePath
      {
        // Properties
        strokeColor: Colors.border
        strokeWidth: Metrics.borderThick
        fillColor: "transparent"
        capStyle: ShapePath.RoundCap
        strokeStyle: ShapePath.DashLine
        // The pattern covers the whole border, so that exactly one segment travels along it.
        dashPattern: dashedBorder.dashPattern

        // Components
        PathRectangle
        {
          // Properties
          // The stroke is centered on the path, so it is inset to stay inside the button.
          x: Metrics.borderThick / 2
          y: Metrics.borderThick / 2
          width: content.width - Metrics.borderThick
          height: content.height - Metrics.borderThick
          radius: Metrics.radiusLarge
        }

        // Animations
        NumberAnimation on dashOffset
        {
          running: root.visible && root.loading
          loops: Animation.Infinite
          from: 0
          to: dashedBorder.period
          duration: Metrics.durationLong
        }
      }
    }

    MouseArea
    {
      id: mouseArea

      // Properties
      anchors.fill: parent
      enabled: !root.loading
      hoverEnabled: true
      cursorShape: Qt.PointingHandCursor

      // Connections
      onClicked: { root.clicked() }
    }

    Timer
    {
      // Properties
      interval: root.timeout
      running: root.shown && !root.loading && !mouseArea.containsMouse

      // Connections
      onTriggered: { root.timedOut() }
    }
  }

  // Functions
  ///
  /// Puts the button onto the area it shall cover. Only a shown button that knows the reference
  /// grows onto it, a hidden one and one reporting a new search jump to their area instead of
  /// flying there from the reference of the previous search.
  ///
  function applyOverlayRect()
  {
    growAnimation.stop()
    if(root.visible && !root.loading)
    {
      growAnimation.to = root.overlayRect
      growAnimation.start()
    }
    else
    {
      /*no binding*/ root.currentRect = root.overlayRect
    }
  }
}
