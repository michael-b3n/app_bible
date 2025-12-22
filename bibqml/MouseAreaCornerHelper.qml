import QtQuick

MouseArea
{
  id: root

  required property int deltaXMultiplier
  required property int deltaYMultiplier
  required property int deltaWidthMultiplier
  required property int deltaHeightMultiplier

  property int clickX: 0
  property int clickY: 0

  signal resizeRequested(deltaX: int, deltaY: int, deltaWidth: int, deltaHeight: int)

  hoverEnabled: true

  onPressed: (mouse) =>
  {
    clickX = mouse.x
    clickY = mouse.y
  }

  onPositionChanged: (mouse) =>
  {
    if(pressed)
    {
      let deltaX = mouse.x - clickX
      let deltaY = mouse.y - clickY
      root.resizeRequested(
        deltaX * deltaXMultiplier,
        deltaY * deltaYMultiplier,
        deltaX * deltaWidthMultiplier,
        deltaY * deltaHeightMultiplier
      )
    }
  }
}
