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
  visible: root.paramControl.hovered
  opacity: root.visible ? 1 : 0

  // Animations
  Behavior on opacity { NumberAnimation { duration: Metrics.durationMedium } }
}
