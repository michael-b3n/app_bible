import QtQuick
import QtQuick.Controls

///
/// Remove button for a single entry of an editable param list.
/// The button is overlaid on the content item of the given param
/// control and is only shown while that control is hovered.
///
ButtonIconSimple
{
  id: root

  // Properties
  required property Control paramControl

  svgSource: Icons.remove

  anchors.right: root.paramControl.contentItem.right
  anchors.rightMargin: Metrics.paddingParamContent
  anchors.verticalCenter: root.paramControl.contentItem.verticalCenter
  width: Metrics.controlHeight - Metrics.spacingTiny
  height: Metrics.controlHeight - Metrics.spacingTiny
  // The button is kept for as long as it is drawn, so that it fades out instead of vanishing
  // the moment the control loses the hover.
  opacity: root.paramControl.hovered ? 1 : 0
  visible: root.opacity > 0

  // Animations
  Behavior on opacity { NumberAnimation { duration: Metrics.durationMedium } }
}
