import QtQuick
import BibQml

///
/// Area the main window covers, together with the math that puts it there.
///
/// The area is a plain property that nothing but the functions below write. The window therefore
/// never moves on its own, it moves exactly when its owner calls one of them, which is what lets
/// the owner keep it from moving in front of the user.
///
/// A pinned window stays where the user put it and only follows the borders of its screen, an
/// unpinned one follows the cursor of the search, keeping the offset the user dragged it to.
///
QtObject
{
  id: root

  // Properties
  // Cursor position an unpinned window is placed relative to, and the screen it is placed on
  required property point cursorPosition
  required property rect cursorScreenGeometry
  // Area an unpinned window steps aside for and the distance it keeps to it. An empty area
  // blocks nothing.
  required property rect blockedArea
  required property real blockedClearance
  // Length of the tail pointing at the window, it is kept free below a window never dragged yet
  required property int tailLength

  // Constants
  readonly property real goldenRatio: 1.618
  readonly property int minimalWidth: Metrics.controlHeight * root.goldenRatio * 3
  readonly property int minimalHeight: Metrics.controlHeight + 2 * Metrics.spacingSmall
  readonly property size defaultSize: Qt.size(root.minimalWidth * 2, root.minimalHeight * 10)

  // Settings, the internal path is not listed in the settings tab
  readonly property SettingBinding settingPinned: BridgeSettings.binding("internal.bubble.pinned", false)
  readonly property SettingBinding settingPinnedX: BridgeSettings.binding("internal.bubble.pinned_x", 0)
  readonly property SettingBinding settingPinnedY: BridgeSettings.binding("internal.bubble.pinned_y", 0)

  // Pinned state, the position is stored to survive a restart
  readonly property bool pinned: root.settingPinned.value
  property int pinnedX: root.settingPinnedX.value
  property int pinnedY: root.settingPinnedY.value

  // Area the window covers
  property rect area: Qt.rect(0, 0, root.defaultSize.width, root.defaultSize.height)

  // Offset to the cursor the user dragged the window to. It starts out above the cursor, leaving
  // room for the tail below the window.
  property point offsetToCursor: Qt.point(
    -root.minimalWidth / root.goldenRatio, -(root.defaultSize.height + root.tailLength)
  )

  // Screen the window is on
  readonly property rect screenGeometry: Placement.screenGeometryOf(root.area)

  // Screen a placement puts the window on: the one the user is looking at, or the pinned one
  readonly property rect placementScreenGeometry: root.pinned
    ? Placement.screenGeometryAt(Qt.point(root.pinnedX, root.pinnedY))
    : root.cursorScreenGeometry

  // Functions
  ///
  /// Places the window.
  ///
  function place()
  {
    if(root.pinned)
    {
      root.applyArea(
        Placement.insideScreen(
          Qt.rect(root.pinnedX, root.pinnedY, root.area.width, root.area.height), root.placementScreenGeometry
        )
      )
      return
    }
    root.applyArea(
      Placement.placedBeside(
        Qt.rect(
          root.cursorPosition.x + root.offsetToCursor.x,
          root.cursorPosition.y + root.offsetToCursor.y,
          root.area.width,
          root.area.height
        ),
        root.blockedArea,
        root.blockedClearance,
        root.placementScreenGeometry
      )
    )
  }

  ///
  /// Moves the window by the deltas the user dragged it.
  ///
  function moveBy(deltaX, deltaY)
  {
    root.applyUserArea(Qt.rect(root.area.x + deltaX, root.area.y + deltaY, root.area.width, root.area.height))
  }

  ///
  /// Resizes the window by the deltas the user dragged it, keeping it on the screen and at its
  /// minimal size.
  ///
  function resizeBy(deltaX, deltaY, deltaWidth, deltaHeight)
  {
    root.applyUserArea(
      Placement.clippedToScreen(
        Qt.rect(
          root.area.x + deltaX, root.area.y + deltaY, root.area.width + deltaWidth, root.area.height + deltaHeight
        ),
        root.screenGeometry,
        root.minimalWidth,
        root.minimalHeight
      )
    )
  }

  ///
  /// Pins the window at its current position, or releases it back to the cursor.
  ///
  function setPinned(pinned)
  {
    if(pinned)
    {
      /*no binding*/ root.pinnedX = root.area.x
      /*no binding*/ root.pinnedY = root.area.y
      root.storePinnedPosition()
    }
    root.settingPinned.value = pinned
  }

  ///
  /// Stores the position the window is pinned at.
  ///
  function storePinnedPosition()
  {
    root.settingPinnedX.value = root.pinnedX
    root.settingPinnedY.value = root.pinnedY
  }

  ///
  /// Puts the window onto the area the user requested, it is only kept on the screen. The area is
  /// remembered unconstrained, so the window returns to it once there is space again.
  ///
  function applyUserArea(target)
  {
    if(!root.pinned)
    {
      /*no binding*/ root.offsetToCursor = Qt.point(
        target.x - root.cursorPosition.x, target.y - root.cursorPosition.y
      )
    }
    root.applyArea(Placement.insideScreen(target, root.screenGeometry))
  }

  ///
  /// Puts the window onto the area, remembering the position a pinned one is kept at.
  ///
  function applyArea(target)
  {
    /*no binding*/ root.area = target
    if(root.pinned)
    {
      /*no binding*/ root.pinnedX = target.x
      /*no binding*/ root.pinnedY = target.y
    }
  }
}
