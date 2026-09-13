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
  // Segments the setting is named by, the first one being the category it is grouped under
  required property var categories
  required property int valueType
  required property int wrapperType
  required property int validatorType
  required property var value
  required property var listValidatorData

  property bool showTitle: true
  readonly property int titleHeight: title.implicitHeight
  readonly property int titleWidth: title.contentWidth + 2 * Metrics.spacingTiny

  topPadding: root.showTitle ? (root.titleHeight + 2 * Metrics.spacingTiny) : Metrics.spacingTiny
  bottomPadding: Metrics.spacingTiny
  leftPadding: Metrics.spacingTiny
  rightPadding: Metrics.spacingTiny

  // Signals
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
    // The category is already named by the section the setting is listed under, so only the
    // segments below it are left to name here. A setting without a category names itself.
    // Note the language is passed to reevaluate this binding on a language change.
    text: Translations.names(root.categories.length > 1 ? root.categories.slice(1) : root.categories,
                             Translations.language)

    visible: root.showTitle
    width: root.width
    // height is implicitly determined
  }
}
