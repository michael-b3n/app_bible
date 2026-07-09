import QtQuick
import QtQuick.Controls

///
/// Parameter base object.
/// Fixed is only the title of the param.
/// Param implementations shall add the according
/// design using the required properties.
///
Control
{
  id: root

  // Properties
  required property string path
  required property int valueType
  required property int wrapperType
  required property int validatorType
  required property var value
  required property var listValidatorData

  property bool showTitle: true
  readonly property int titleHeight: title.implicitHeight
  readonly property int titleWidth: title.contentWidth + 2 * Metrics.spacingTiny

  topPadding: showTitle ? (titleHeight + 2 * Metrics.spacingTiny) : Metrics.spacingTiny
  bottomPadding: Metrics.spacingTiny
  leftPadding: Metrics.spacingTiny
  rightPadding: Metrics.spacingTiny


  ///
  /// The signal will notify the backend about the change,
  /// the value will be validated and loop back to UI
  /// if there was an error or the desired value was not set.
  ///
  signal paramValueChanged(value: var)

  // Components
  ///
  /// Title of the param object.
  ///
  ParamText
  {
    id: title

    // Properties
    text: root.path

    visible: root.showTitle
    width: root.width
    // height is implicitly determined
  }
}
