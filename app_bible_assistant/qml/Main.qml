import QtQuick
import BibQml

///
/// Application root object.
/// It owns the transparent background window carrying the speech bubble
/// shape and the main window carrying the actual content. Both windows are
/// positioned relative to the cursor position reported by the ocr bridge.
///
/// Note the /*no binding*/ marker below flags imperative assignments that
/// deliberately overwrite a binding.
///
QtObject
{
  id: root

  // Properties
  required property SettingsListModel listModelSettings
  required property ScriptureListModel listModelScripture
  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property BridgeBibleRefLookup bridgeBibleRefLookup

  // Constants
  readonly property real goldenRatio: 1.618
  readonly property int margin: Metrics.spacingSmall
  readonly property int radius: Metrics.radiusLarge
  readonly property int minimalWidth: Metrics.controlHeight * root.goldenRatio * 3
  readonly property int minimalHeight: Metrics.controlHeight + 2 * root.margin
  // Time the overlay of a found reference stays on the screen while it is not hovered.
  readonly property int referenceOverlayTimeout: 10000
  // Margin the overlay of a found reference adds around the reference text. This makes the
  // overlay easier to hit than the tight bounding box of the text.
  readonly property int referenceOverlayMargin: Metrics.spacingMedium
  // Distance between the cursor and the bubble, this is the length of the bubble tail.
  readonly property int tailLength: Metrics.spacingLarge * 2

  // Screen geometry
  property var screenGeometry: ScreenGeometryHelper.screenGeometryAt({ x: 0, y: 0 })
  readonly property int screenLeftBorder: root.screenGeometry.x
  readonly property int screenRightBorder: root.screenGeometry.x + root.screenGeometry.width
  readonly property int screenTopBorder: root.screenGeometry.y
  readonly property int screenBottomBorder: root.screenGeometry.y + root.screenGeometry.height

  // Cursor position
  property int cursorX: 0
  property int cursorY: 0

  // Reference found on the screen. The rect is the area the reference text
  // covers on the screen, it is empty if that area is unknown.
  property string referenceBookId: ""
  property int referenceChapterBegin: 0
  property int referenceVerseBegin: 0
  property int referenceChapterEnd: 0
  property int referenceVerseEnd: 0
  property rect referenceRect: Qt.rect(0, 0, 0, 0)

  // Bubble offset and size
  property int userOffsetToCursorX: -root.minimalWidth / root.goldenRatio
  property int userOffsetToCursorY: -(root.minimalHeight * 10 + root.tailLength)
  property int offsetToCursorX: -Metrics.controlHeight * root.goldenRatio
  property int offsetToCursorY: -100
  property int mainWidth: root.minimalWidth * 2
  property int mainHeight: root.minimalHeight * 10

  // Connections
  property Connections bridgeConnections: Connections
  {
    target: root.bridgeBibleRefOcr

    function onCursorPositionChanged(cursorPosition)
    {
      root.cursorX = cursorPosition.x
      root.cursorY = cursorPosition.y
      root.screenGeometry = ScreenGeometryHelper.screenGeometryAt(cursorPosition)
      Qt.callLater(function() { root.applyConstrainedOffset() })
    }

    function onReferenceRangeFound(bookId, chapterBegin, verseBegin, chapterEnd, verseEnd, boundingBox)
    {
      root.referenceBookId = bookId
      root.referenceChapterBegin = chapterBegin
      root.referenceVerseBegin = verseBegin
      root.referenceChapterEnd = chapterEnd
      root.referenceVerseEnd = verseEnd
      root.referenceRect = boundingBox
    }

    function onRunningChanged(running)
    {
      Qt.callLater(function()
      {
        if(running)
        {
          // The reference of the previous run is not valid anymore.
          root.clearReference()
          background.raise()
          root.show()
        }
        else if(!mouseAreaHelper.containsMouse)
        {
          // TODO hiding on a stopped ocr run is disabled until the
          // auto hide behaviour is settled, see root.hide().
        }
      })
    }
  }

  // Background window with speech bubble shape
  property Window background: Window
  {
    id: background

    // Properties
    visible: speechBubble.opacity > 0
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.WindowTransparentForInput
    x: root.screenGeometry.x
    y: root.screenGeometry.y
    width: root.screenGeometry.width
    height: root.screenGeometry.height

    // Connections
    onVisibleChanged: { if(background.visible) { Qt.callLater(function() { main.raise() }) } }

    // Components
    SpeechBubbleShape
    {
      id: speechBubble

      // Properties
      opacity: 0
      radius: root.radius
      tailPositionX: root.cursorX - background.x
      tailPositionY: root.cursorY - background.y
      offsetToTailX: root.offsetToCursorX
      offsetToTailY: root.offsetToCursorY
      bubbleWidth: root.mainWidth
      bubbleHeight: root.mainHeight
      strokeColor: Colors.border
      fillColor: Colors.backgroundTransparent

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
  }

  ///
  /// Transparent button covering the reference found on the screen. Clicking it
  /// executes the action the user configured for found references.
  ///
  property Window referenceOverlay: Window
  {
    id: referenceOverlay

    // Properties
    color: "transparent"
    // The overlay must not take the focus away from the window the user is reading.
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool | Qt.WindowDoesNotAcceptFocus
    visible: root.referenceRect.width > 0 && root.referenceRect.height > 0
    x: root.referenceRect.x - root.referenceOverlayMargin
    y: root.referenceRect.y - root.referenceOverlayMargin
    width: root.referenceRect.width + 2 * root.referenceOverlayMargin
    height: root.referenceRect.height + 2 * root.referenceOverlayMargin

    // Components
    Rectangle
    {
      // Properties
      anchors.fill: parent
      // The reference below the overlay shall stay readable, so the area is only
      // tinted while it is hovered. Note that the opacity never drops to zero:
      // fully transparent pixels of a translucent window are not hit by mouse
      // events, which would make the overlay unclickable.
      color: Colors.selection
      opacity: overlayMouseArea.containsMouse ? 0.3 : 0.02
      border.color: Colors.border
      border.width: overlayMouseArea.containsMouse ? Metrics.border : 0
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

    MouseArea
    {
      id: overlayMouseArea

      // Properties
      anchors.fill: parent
      hoverEnabled: true
      cursorShape: Qt.PointingHandCursor

      // Connections
      onClicked: { root.triggerReferenceClickAction() }
    }

    ///
    /// Hides the overlay if it is not used for a while. The content below the
    /// overlay may have scrolled away or changed in the meantime.
    ///
    Timer
    {
      // Properties
      interval: root.referenceOverlayTimeout
      running: referenceOverlay.visible && !overlayMouseArea.containsMouse

      // Connections
      onTriggered: { root.clearReference() }
    }
  }

  // Main window with simple rounded rectangle shape
  property Window main: Window
  {
    id: main

    // Properties
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    opacity: speechBubble.opacity
    visible: speechBubble.opacity > 0
    x: root.cursorX + root.offsetToCursorX
    y: root.cursorY + root.offsetToCursorY
    width: root.mainWidth
    height: root.mainHeight

    // Components
    // Mouse area for interaction
    MouseAreaHelper
    {
      id: mouseAreaHelper

      // Properties
      anchors.fill: parent
      expandable: true
      expandAreaWidth: Metrics.spacingMedium
      movable: true

      // Connections
      onReleased: { root.show() }
      onMoveRequested: (deltaX, deltaY) =>
      {
        root.userOffsetToCursorX = root.offsetToCursorX + deltaX
        root.userOffsetToCursorY = root.offsetToCursorY + deltaY
        root.applyConstrainedOffset()
      }
      onExpandRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
      {
        root.userOffsetToCursorX = root.offsetToCursorX + deltaX
        root.userOffsetToCursorY = root.offsetToCursorY + deltaY
        let constrained = root.constrainSize(
          root.userOffsetToCursorX,
          root.userOffsetToCursorY,
          root.mainWidth + deltaWidth,
          root.mainHeight + deltaHeight
        )
        /*no binding*/ root.offsetToCursorX = constrained.x
        /*no binding*/ root.offsetToCursorY = constrained.y
        /*no binding*/ root.mainWidth = constrained.width
        /*no binding*/ root.mainHeight = constrained.height
      }

      // Components
      MainTabLayout
      {
        id: mainTabLayout

        // Properties
        listModelSettings: root.listModelSettings
        listModelScripture: root.listModelScripture
        bridgeBibleRefOcr: root.bridgeBibleRefOcr
        bridgeBibleRefLookup: root.bridgeBibleRefLookup

        anchors.fill: parent

        // Connections
        onCloseClicked: () =>
        {
          Qt.callLater(function()
          {
            root.hide()
          })
        }
      }
    }
  }

  // Functions
  ///
  /// Fades both windows in and raises the main window above the background.
  /// Both windows follow the bubble opacity, so this shows them as well.
  ///
  function show()
  {
    speechBubble.opacity = 1
    Qt.callLater(function() { main.raise() })
  }

  ///
  /// Fades both windows out.
  /// Both windows follow the bubble opacity, so this hides them as well.
  ///
  function hide()
  {
    speechBubble.opacity = 0
    root.clearReference()
  }

  ///
  /// Clears the reference found on the screen, which hides its overlay.
  ///
  function clearReference()
  {
    root.referenceRect = Qt.rect(0, 0, 0, 0)
  }

  ///
  /// Executes the action the user configured for a click on a found reference.
  /// Every action maps to a capability of the frontend, so the dispatch is done here.
  ///
  function triggerReferenceClickAction()
  {
    switch(root.bridgeBibleRefOcr.clickAction())
    {
      case BridgeBibleRefOcr.LookupBrowser:
      {
        root.bridgeBibleRefLookup.lookup(
          root.referenceBookId,
          root.referenceChapterBegin,
          root.referenceVerseBegin,
          root.referenceChapterEnd,
          root.referenceVerseEnd
        )
        break
      }
      case BridgeBibleRefOcr.None: break
    }
  }

  ///
  /// Constrains the user requested offset to the current screen
  /// and applies it to the offset the windows are positioned by.
  ///
  function applyConstrainedOffset()
  {
    let constrained = root.constrainOffset(
      root.userOffsetToCursorX,
      root.userOffsetToCursorY,
      root.mainWidth,
      root.mainHeight
    )
    /*no binding*/ root.offsetToCursorX = constrained.x
    /*no binding*/ root.offsetToCursorY = constrained.y
  }

  ///
  /// Constrains the bubble offset to keep it within screen bounds.
  /// Returns adjusted offset coordinates that ensure the bubble stays fully visible.
  ///
  function constrainOffset(newOffsetX, newOffsetY, width, height)
  {
    let bubbleLeft = root.cursorX + newOffsetX
    let bubbleTop = root.cursorY + newOffsetY
    let bubbleRight = bubbleLeft + width
    let bubbleBottom = bubbleTop + height

    if(bubbleLeft < root.screenLeftBorder)
    {
      newOffsetX = root.screenLeftBorder - root.cursorX
    }
    else if(bubbleRight > root.screenRightBorder)
    {
      newOffsetX = root.screenRightBorder - width - root.cursorX
    }

    if(bubbleTop < root.screenTopBorder)
    {
      newOffsetY = root.screenTopBorder - root.cursorY
    }
    else if(bubbleBottom > root.screenBottomBorder)
    {
      newOffsetY = root.screenBottomBorder - height - root.cursorY
    }
    return {x: newOffsetX, y: newOffsetY}
  }

  ///
  /// Constrains the bubble size and offset during resize operations.
  /// Ensures the bubble stays within screen bounds and maintains minimum size.
  /// Returns adjusted offset and size that respect screen boundaries.
  ///
  function constrainSize(newOffsetX, newOffsetY, newWidth, newHeight)
  {
    let bubbleLeft = root.cursorX + newOffsetX
    let bubbleTop = root.cursorY + newOffsetY
    let bubbleRight = bubbleLeft + newWidth
    let bubbleBottom = bubbleTop + newHeight

    if(bubbleLeft < root.screenLeftBorder)
    {
      newWidth -= (root.screenLeftBorder - bubbleLeft)
      newOffsetX = root.screenLeftBorder - root.cursorX
    }
    if(bubbleTop < root.screenTopBorder)
    {
      newHeight -= (root.screenTopBorder - bubbleTop)
      newOffsetY = root.screenTopBorder - root.cursorY
    }
    if(bubbleRight > root.screenRightBorder)
    {
      newWidth = root.screenRightBorder - (root.cursorX + newOffsetX)
    }
    if(bubbleBottom > root.screenBottomBorder)
    {
      newHeight = root.screenBottomBorder - (root.cursorY + newOffsetY)
    }
    // Enforce minimum size
    newWidth = Math.max(newWidth, root.minimalWidth)
    newHeight = Math.max(newHeight, root.minimalHeight)
    return {x: newOffsetX, y: newOffsetY, width: newWidth, height: newHeight}
  }

}
