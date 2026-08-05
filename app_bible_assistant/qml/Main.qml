import QtQuick
import BibQml

///
/// Application root object.
/// It owns the main window carrying the content, the bubble window drawing the frame around it and
/// the overlay button that reports a running search and covers the reference it found.
///
/// A search replaces what the main window shows, so the window disappears when one starts and only
/// comes back with its result. On its way back it passes the placing phase, which is what keeps it
/// from moving in front of the user: it is placed while it is still hidden, and a window that has
/// not disappeared yet waits until it did.
///
QtObject
{
  id: root

  // Typedefs
  ///
  /// Phases the application goes through. They describe what is on the screen, the main window is
  /// shown in the shown phase only.
  ///
  enum Phase
  {
    // Nothing is on the screen
    Hidden,
    // A search runs and reports itself by the overlay, the main window is hidden
    Searching,
    // The main window is asked for and waits until it can be placed
    Placing,
    // The main window is on the screen
    Shown
  }

  // Properties
  required property SettingsListModel listModelSettings
  required property ScriptureListModel listModelScripture
  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property BridgeBibleRefLookup bridgeBibleRefLookup
  required property BridgeApplication bridgeApplication

  // Constants
  readonly property int referenceOverlayTimeout: 10000
  // Size the overlay has while the area of the reference is still unknown
  readonly property int overlayDefaultSize: Metrics.controlHeight + 2 * Metrics.spacingSmall
  // Length of the bubble tail
  readonly property int tailLength: Metrics.spacingLarge * 2
  // Gap between the tail tip and the reference overlay, so the tail does not cover it
  readonly property int referenceTailGap: Metrics.spacingSmall

  // Phase the application is in
  property int phase: Main.Phase.Hidden
  readonly property bool searching: root.phase === Main.Phase.Searching
  readonly property bool windowShown: root.phase === Main.Phase.Shown

  // Cursor position of the running or last search
  property point cursorPosition: Qt.point(0, 0)
  property rect cursorScreenGeometry: Placement.screenGeometryAt(Qt.point(0, 0))

  // Reference of the last search
  property QtObject reference: QtObject
  {
    id: reference

    // Properties
    property string bookId: ""
    property int chapterBegin: 0
    property int verseBegin: 0
    property int chapterEnd: 0
    property int verseEnd: 0
    // Area the reference covers on the screen, it is empty if the search could not tell where it is
    property rect area: Qt.rect(0, 0, 0, 0)

    readonly property bool areaKnown: reference.area.width > 0 && reference.area.height > 0
    // Area the overlay covers, the margin around the reference text makes it easier to hit
    readonly property rect overlayArea: Placement.grown(reference.area, Metrics.spacingMedium)

    // Functions
    ///
    /// Forgets where the reference is on the screen, which hides its overlay.
    ///
    function forgetArea()
    {
      /*no binding*/ reference.area = Qt.rect(0, 0, 0, 0)
    }
  }

  // Overlay button state: it reports the running search until the area it covers is known, and it
  // has its default size at the cursor as long as that area is unknown
  readonly property bool overlayLoading: root.searching && !reference.areaKnown
  readonly property bool overlayShown: root.searching || reference.areaKnown
  readonly property rect overlayRect: reference.areaKnown
    ? reference.overlayArea
    : Placement.centeredSquare(root.cursorPosition, root.overlayDefaultSize, root.cursorScreenGeometry)

  // Point the tail points at: the border of the reference overlay, or the cursor
  readonly property point tailPosition: reference.areaKnown
    ? Placement.borderPointTowards(reference.overlayArea, root.referenceTailGap, mainPlacement.area)
    : root.cursorPosition

  // Placement
  property MainWindowPlacement mainPlacement: MainWindowPlacement
  {
    id: mainPlacement

    // Properties
    cursorPosition: root.cursorPosition
    cursorScreenGeometry: root.cursorScreenGeometry
    // The main window steps aside for the reference overlay, leaving room for the tail
    blockedArea: reference.overlayArea
    blockedClearance: root.tailLength + root.referenceTailGap
    tailLength: root.tailLength
  }

  // Connections
  property Connections bridgeConnections: Connections
  {
    target: root.bridgeBibleRefOcr

    function onCursorPositionChanged(cursorPosition)
    {
      root.cursorPosition = cursorPosition
      root.cursorScreenGeometry = Placement.screenGeometryAt(cursorPosition)
    }

    function onReferenceRangeFound(bookId, chapterBegin, verseBegin, chapterEnd, verseEnd, boundingBox)
    {
      reference.bookId = bookId
      reference.chapterBegin = chapterBegin
      reference.verseBegin = verseBegin
      reference.chapterEnd = chapterEnd
      reference.verseEnd = verseEnd
      reference.area = boundingBox
      // The window shows the passage of this reference and steps aside for it, so it is placed
      // again even if it happens to be on the screen already.
      root.placeAndShowWindow()
    }

    function onRunningChanged(running)
    {
      if(running)
      {
        root.beginSearch()
      }
      else
      {
        // The result follows this notification, so the phase is only settled afterwards.
        Qt.callLater(root.finishSearch)
      }
    }
  }

  property Connections applicationConnections: Connections
  {
    target: root.bridgeApplication

    function onShowWindowRequested() { root.showWindow() }
  }

  // Windows
  property Window bubble: SpeechBubbleWindow
  {
    id: bubble

    // Properties
    screenGeometry: mainPlacement.screenGeometry
    bubbleRect: mainPlacement.area
    tailPosition: root.tailPosition
    // A pinned main window does not belong to a position on the screen, so it shows no tail.
    tailVisible: !mainPlacement.pinned
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
    onTimedOut: { reference.forgetArea() }
  }

  property Window main: MainWindow
  {
    id: main

    // Properties
    listModelSettings: root.listModelSettings
    listModelScripture: root.listModelScripture
    bridgeBibleRefOcr: root.bridgeBibleRefOcr
    bridgeBibleRefLookup: root.bridgeBibleRefLookup
    mainRect: mainPlacement.area
    pinned: mainPlacement.pinned
    shown: root.windowShown

    // Connections
    onVisibleChanged:
    {
      // A hidden window is free to move, so a window waiting for its placement gets it now.
      Qt.callLater(root.placeWindow)
      Qt.callLater(root.raiseWindows)
    }
    onReleased:
    {
      root.raiseWindows()
      if(mainPlacement.pinned)
      {
        mainPlacement.storePinnedPosition()
      }
    }
    onMoveRequested: (deltaX, deltaY) => { mainPlacement.moveBy(deltaX, deltaY) }
    onExpandRequested: (deltaX, deltaY, deltaWidth, deltaHeight) =>
    {
      mainPlacement.resizeBy(deltaX, deltaY, deltaWidth, deltaHeight)
    }
    onCloseClicked: { Qt.callLater(root.hideWindow) }
    onPinClicked: { mainPlacement.setPinned(!mainPlacement.pinned) }
  }

  // Functions
  ///
  /// Begins a search. The passage the main window shows belongs to the previous one, so the window
  /// disappears until this search has a result.
  ///
  function beginSearch()
  {
    root.phase = Main.Phase.Searching
    reference.forgetArea()
  }

  ///
  /// Ends the running search. A search that found a reference asked for the main window already,
  /// one without a result leaves it hidden, there is nothing to show anymore.
  ///
  function finishSearch()
  {
    if(root.phase === Main.Phase.Searching)
    {
      root.phase = Main.Phase.Hidden
    }
  }

  ///
  /// Places the main window and shows it there. Only a hidden window may be placed, so a window
  /// that is still on the screen disappears first and is placed once it did.
  ///
  function placeAndShowWindow()
  {
    root.phase = Main.Phase.Placing
    root.placeWindow()
  }

  ///
  /// Shows the main window. A window that is on the screen already stays where it is.
  ///
  function showWindow()
  {
    if(root.phase !== Main.Phase.Shown)
    {
      root.placeAndShowWindow()
    }
  }

  ///
  /// Hides the main window and the overlay of the found reference.
  ///
  function hideWindow()
  {
    root.phase = Main.Phase.Hidden
    reference.forgetArea()
  }

  ///
  /// Places the main window waiting for it and shows it, as soon as it is hidden.
  ///
  function placeWindow()
  {
    if(root.phase !== Main.Phase.Placing || main.visible)
    {
      return
    }
    mainPlacement.place()
    root.phase = Main.Phase.Shown
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
  /// Executes the action the user configured for a click on a found reference.
  ///
  function triggerReferenceClickAction()
  {
    switch(root.bridgeBibleRefOcr.clickAction())
    {
      case BridgeBibleRefOcr.LookupBrowser:
      {
        root.bridgeBibleRefLookup.lookup(
          reference.bookId, reference.chapterBegin, reference.verseBegin, reference.chapterEnd, reference.verseEnd
        )
        break
      }
      case BridgeBibleRefOcr.None: break
    }
  }
}
