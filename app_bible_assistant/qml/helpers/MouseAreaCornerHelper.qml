import QtQuick

///
/// Mouse area of a single resize corner.
/// The multipliers describe how a drag of this corner
/// translates into an offset and size change of the target.
///
MouseArea
{
  id: root

  // Properties
  required property int deltaXMultiplier
  required property int deltaYMultiplier
  required property int deltaWidthMultiplier
  required property int deltaHeightMultiplier

  property int clickX: 0
  property int clickY: 0

  hoverEnabled: true

  // Signals
  signal resizeRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)

  // Connections
  onPressed: (mouse) =>
  {
    root.clickX = mouse.x
    root.clickY = mouse.y
  }

  onPositionChanged: (mouse) =>
  {
    if(root.pressed)
    {
      let deltaX = mouse.x - root.clickX
      let deltaY = mouse.y - root.clickY
      root.resizeRequested(
        deltaX * root.deltaXMultiplier,
        deltaY * root.deltaYMultiplier,
        deltaX * root.deltaWidthMultiplier,
        deltaY * root.deltaHeightMultiplier
      )
    }
  }
}
