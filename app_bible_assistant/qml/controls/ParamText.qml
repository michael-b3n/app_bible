import QtQuick

///
/// Parameter text object with default properties.
/// Height shall be left untouched to be determined
/// by the content. Width has to be set.
///
Text
{
  id: root

  // Properties
  font.pointSize: Metrics.fontSizeParam
  color: Colors.text
  renderType: Text.CurveRendering
  verticalAlignment: Text.AlignVCenter
}
