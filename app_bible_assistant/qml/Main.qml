import QtQuick
import BibQml

///
/// Application root object.
/// It owns the bubble window drawing the frame around the main window carrying the content, and
/// the overlay button that reports a running search and covers the reference it found.
///
/// A search replaces what the main window shows, so the window disappears when one starts and only
/// comes back with its result. It is placed while it is hidden, which is what keeps it from moving
/// in front of the user: the area it covers is a plain property that nothing but the placement
/// functions write, and a placement requested while the window is still shown waits until it
/// disappeared.
///
QtObject
{
  id: root

  // Properties
  required property SettingsListModel listModelSettings
  required property ScriptureListModel listModelScripture
  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property BridgeBibleRefLookup bridgeBibleRefLookup
  required property BridgeApplication bridgeApplication

  // Constants
  readonly property real goldenRatio: 1.618
  readonly property int margin: Metrics.spacingSmall
  readonly property int minimalWidth: Metrics.controlHeight * root.goldenRatio * 3
  readonly property int minimalHeight: Metrics.controlHeight + 2 * root.margin
  readonly property int referenceOverlayTimeout: 10000
  // Margin around the reference text, it makes the overlay easier to hit.
  readonly property int referenceOverlayMargin: Metrics.spacingMedium
  // Size the overlay has while the area of the reference is still unknown.
  readonly property int overlayDefaultSize: root.minimalHeight
  // Length of the bubble tail.
  readonly property int tailLength: Metrics.spacingLarge * 2
  // Gap between the tail tip and the reference overlay, so the tail does not cover it.
  readonly property int referenceTailGap: Metrics.spacingSmall
  // Distance the main window keeps to the reference overlay, it leaves room for the tail.
  readonly property int referenceClearance: root.tailLength + root.referenceTailGap

  // Settings, the internal path is not listed in the settings tab
  readonly property SettingBinding settingPinned: BridgeSettings.binding("internal.bubble.pinned", false)
  readonly property SettingBinding settingPinnedX: BridgeSettings.binding("internal.bubble.pinned_x", 0)
  readonly property SettingBinding settingPinnedY: BridgeSettings.binding("internal.bubble.pinned_y", 0)

  // Cursor position of the running or last search
  property int cursorX: 0
  property int cursorY: 0
  property rect cursorScreenGeometry: Placement.screenGeometryAt(Qt.point(0, 0))

  // Pinned state, the position is stored to survive a restart
  readonly property bool pinned: root.settingPinned.value
  property int pinnedX: root.settingPinnedX.value
  property int pinnedY: root.settingPinnedY.value

  // Reference of the last search, its rect is empty if the area it covers is unknown
  property string referenceBookId: ""
  property int referenceChapterBegin: 0
  property int referenceVerseBegin: 0
  property int referenceChapterEnd: 0
  property int referenceVerseEnd: 0
  property rect referenceRect: Qt.rect(0, 0, 0, 0)
  readonly property bool referenceAreaKnown: root.referenceRect.width > 0 && root.referenceRect.height > 0
  readonly property rect referenceOverlayRect: Placement.grown(root.referenceRect, root.referenceOverlayMargin)

  // Search and window state. The main window only opens on its own if a search found a reference,
  // and a window waiting for its placement stays hidden until it got it.
  property bool searching: false
  property bool windowRequested: false
  property bool placementPending: false
  readonly property bool windowShown: root.windowRequested && !root.placementPending

  // Overlay button state: it reports the running search until the area it covers is known
  readonly property bool overlayLoading: root.searching && !root.referenceAreaKnown
  readonly property bool overlayShown: root.searching || root.referenceAreaKnown
  readonly property rect overlayRect: root.referenceAreaKnown
    ? root.referenceOverlayRect
    : Placement.centeredSquare(
        Qt.point(root.cursorX, root.cursorY), root.overlayDefaultSize, root.cursorScreenGeometry
      )

  // Area the main window covers and the offset to the cursor the user dragged it to
  property rect mainRect: Qt.rect(0, 0, root.minimalWidth * 2, root.minimalHeight * 10)
  property int userOffsetToCursorX: -root.minimalWidth / root.goldenRatio
  property int userOffsetToCursorY: -(root.minimalHeight * 10 + root.tailLength)

  // Screen the bubble is drawn on, it is the one the main window is on
  readonly property rect screenGeometry: Placement.screenGeometryOf(root.mainRect)

  // Screen a placement puts the main window on: the one the user is looking at, or the pinned one
  readonly property rect placementScreenGeometry: root.pinned
    ? Placement.screenGeometryAt(Qt.point(root.pinnedX, root.pinnedY))
    : root.cursorScreenGeometry

  // Point the tail points at: the border of the reference overlay, or the cursor
  readonly property point tailPosition: root.referenceAreaKnown
    ? Placement.borderPointTowards(root.referenceOverlayRect, root.referenceTailGap, root.mainRect)
    : Qt.point(root.cursorX, root.cursorY)

  // Connections
  Component.onCompleted: { root.requestPlacement() }

  property Connections bridgeConnections: Connections
  {
    target: root.bridgeBibleRefOcr

    function onCursorPositionChanged(cursorPosition)
    {
      root.cursorX = cursorPosition.x
      root.cursorY = cursorPosition.y
      root.cursorScreenGeometry = Placement.screenGeometryAt(cursorPosition)
    }

    function onReferenceRangeFound(bookId, chapterBegin, verseBegin, chapterEnd, verseEnd, boundingBox)
    {
      root.referenceBookId = bookId
      root.referenceChapterBegin = chapterBegin
      root.referenceVerseBegin = verseBegin
      root.referenceChapterEnd = chapterEnd
      root.referenceVerseEnd = verseEnd
      root.referenceRect = boundingBox
      root.openWindow()
    }

    function onRunningChanged(running)
    {
      if(running)
      {
        root.beginSearch()
      }
      else
      {
        // The result follows this notification, so the state is only settled afterwards.
        Qt.callLater(root.finishSearch)
      }
    }
  }

  property Connections applicationConnections: Connections
  {
    target: root.bridgeApplication

    function onShowWindowRequested() { root.openWindow() }
  }

  // Windows
  property Window bubble: SpeechBubbleWindow
  {
    id: bubble

    // Properties
    screenGeometry: root.screenGeometry
    bubbleRect: root.mainRect
    tailPosition: root.tailPosition
    // A pinned main window does not belong to a position on the screen, so it shows no tail.
    tailVisible: !root.pinned
    shown: root.windowShown

    // Connections
    onVisibleChanged: { Qt.callLater(root.raiseWindows) }
  }

  property Window referenceOverlay: ReferenceOverlayWindow
  {
    // Properties
    overlayRect: root.overlayRect
    loading: root.overlayLoading
    shown: root.overlayShown
    timeout: root.referenceOverlayTimeout

    // Connections
    onClicked: { root.triggerReferenceClickAction() }
    onTimedOut: { root.clearReference() }
  }

  property Window main: MainWindow
  {
    id: main

    // Properties
    listModelSettings: root.listModelSettings
    listModelScripture: root.listModelScripture
    bridgeBibleRefOcr: root.bridgeBibleRefOcr
    bridgeBibleRefLookup: root.bridgeBibleRefLookup
    mainRect: root.mainRect
    pinned: root.pinned
    shown: root.windowShown

    // Connections
    onVisibleChanged:
    {
      // A hidden window is free to move, so a placement waiting for it is applied now.
      Qt.callLater(root.applyPendingPlacement)
      Qt.callLater(root.raiseWindows)
    }
    onReleased:
    {
      root.raiseWindows()
      if(root.pinned)
      {
        root.storePinnedPosition()
      }
    }
    onMoveRequested: (deltaX, deltaY) => { root.moveMainBy(deltaX, deltaY) }
    onExpandRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      root.resizeMainBy(deltaX, deltaY, deltaWidth, deltaHeight)
    }
    onCloseClicked: { Qt.callLater(root.closeWindow) }
    onPinClicked: { root.setPinned(!root.pinned) }
  }

  // Functions
  ///
  /// Shows the main window.
  ///
  function openWindow()
  {
    root.windowRequested = true
  }

  ///
  /// Hides the main window and the overlay of the found reference.
  ///
  function closeWindow()
  {
    root.windowRequested = false
    root.clearReference()
  }

  ///
  /// Restores the window order: the bubble is the frame, so it stays below the content.
  ///
  function raiseWindows()
  {
    bubble.raise()
    main.raise()
  }

  ///
  /// Starts a search. The passage the main window shows belongs to the previous one, so the window
  /// disappears until this search has a result. Its placement is requested right away, so a result
  /// arriving before the window disappeared cannot show it at the position of the previous search.
  ///
  function beginSearch()
  {
    root.searching = true
    root.windowRequested = false
    root.clearReference()
    root.requestPlacement()
  }

  ///
  /// Ends the running search and places the main window at what it found. A search without a result
  /// leaves the window hidden, there is nothing to show anymore.
  ///
  function finishSearch()
  {
    root.searching = false
    root.requestPlacement()
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
  /// Pins the main window at its current position, or releases it back to the cursor.
  ///
  function setPinned(pinned)
  {
    if(pinned)
    {
      /*no binding*/ root.pinnedX = root.mainRect.x
      /*no binding*/ root.pinnedY = root.mainRect.y
      root.storePinnedPosition()
    }
    root.settingPinned.value = pinned
  }

  ///
  /// Stores the position the main window is pinned at.
  ///
  function storePinnedPosition()
  {
    root.settingPinnedX.value = root.pinnedX
    root.settingPinnedY.value = root.pinnedY
  }

  ///
  /// Requests a placement of the main window. It is applied at once if the window is hidden,
  /// otherwise the window keeps its area until it disappeared and is placed then.
  ///
  function requestPlacement()
  {
    root.placementPending = true
    root.applyPendingPlacement()
  }

  ///
  /// Applies a requested placement, as soon as the main window is hidden.
  ///
  function applyPendingPlacement()
  {
    if(!root.placementPending || main.visible)
    {
      return
    }
    root.applyPlacement()
    root.placementPending = false
  }

  ///
  /// Places the main window: a pinned one only follows the borders of its screen, an unpinned one
  /// goes where the user dragged it relative to the cursor and steps aside if it would cover the
  /// reference the search found.
  ///
  function applyPlacement()
  {
    if(root.pinned)
    {
      root.applyRect(
        Placement.insideScreen(
          Qt.rect(root.pinnedX, root.pinnedY, root.mainRect.width, root.mainRect.height),
          root.placementScreenGeometry
        )
      )
      return
    }
    root.applyRect(
      Placement.placedBeside(
        Qt.rect(
          root.cursorX + root.userOffsetToCursorX,
          root.cursorY + root.userOffsetToCursorY,
          root.mainRect.width,
          root.mainRect.height
        ),
        root.referenceOverlayRect,
        root.referenceClearance,
        root.placementScreenGeometry
      )
    )
  }

  ///
  /// Moves the main window by the deltas the user dragged it.
  ///
  function moveMainBy(deltaX, deltaY)
  {
    root.applyUserRect(
      Qt.rect(root.mainRect.x + deltaX, root.mainRect.y + deltaY, root.mainRect.width, root.mainRect.height)
    )
  }

  ///
  /// Resizes the main window by the deltas the user dragged it, keeping it on the screen and at its
  /// minimal size.
  ///
  function resizeMainBy(deltaX, deltaY, deltaWidth, deltaHeight)
  {
    root.applyUserRect(
      Placement.clippedToScreen(
        Qt.rect(
          root.mainRect.x + deltaX,
          root.mainRect.y + deltaY,
          root.mainRect.width + deltaWidth,
          root.mainRect.height + deltaHeight
        ),
        root.screenGeometry,
        root.minimalWidth,
        root.minimalHeight
      )
    )
  }

  ///
  /// Puts the main window onto the area the user requested, it is only kept on the screen. The area
  /// is remembered unconstrained, so the window returns to it once there is space again.
  ///
  function applyUserRect(area)
  {
    if(!root.pinned)
    {
      /*no binding*/ root.userOffsetToCursorX = area.x - root.cursorX
      /*no binding*/ root.userOffsetToCursorY = area.y - root.cursorY
    }
    root.applyRect(Placement.insideScreen(area, root.screenGeometry))
  }

  ///
  /// Puts the main window onto the area, remembering the position a pinned one is kept at.
  ///
  function applyRect(area)
  {
    /*no binding*/ root.mainRect = area
    if(root.pinned)
    {
      /*no binding*/ root.pinnedX = area.x
      /*no binding*/ root.pinnedY = area.y
    }
  }
}
