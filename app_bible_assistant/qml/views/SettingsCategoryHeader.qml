import QtQuick

///
/// Header of one settings category. It names the category and folds its settings away when it
/// is clicked, so that the user only reads the settings of the categories they are after.
///
Item
{
  id: root

  // Properties
  // Key of the category, it is named through the translations like any other key
  required property string category
  // Whether the settings of the category are folded away. The header only reports that it was
  // clicked, what is folded away is decided by the list the settings are shown in.
  required property bool collapsed

  implicitHeight: Metrics.controlHeight

  // Signals
  signal toggled()

  // Components
  Rectangle
  {
    id: background

    // Properties
    anchors.fill: parent
    anchors.topMargin: Metrics.spacingTiny
    anchors.bottomMargin: Metrics.spacingTiny
    radius: Metrics.radiusSmall
    color:
    {
      if(mouseArea.pressed) { return Colors.pressed }
      if(mouseArea.containsMouse) { return Colors.selection }
      return Colors.backgroundSolidDarker
    }

    // Components
    ///
    /// Fold indicator. It points down while the category is unfolded, telling that its settings
    /// follow below, and to the right of them while it is folded away.
    ///
    TriangleShape
    {
      id: indicator

      // Properties
      anchors.left: parent.left
      anchors.leftMargin: Metrics.spacingSmall
      anchors.verticalCenter: parent.verticalCenter
      width: Metrics.controlHeight / 2
      height: Metrics.controlHeight / 3
      color: Colors.borderDarker
      transformOrigin: Item.Center
      rotation: root.collapsed ? -90 : 0

      // Animations
      Behavior on rotation
      {
        NumberAnimation
        {
          duration: Metrics.durationShort
          easing.type: Easing.InOutQuad
        }
      }
    }

    ParamText
    {
      // Properties
      anchors.left: indicator.right
      anchors.leftMargin: Metrics.spacingSmall
      anchors.right: parent.right
      anchors.rightMargin: Metrics.spacingSmall
      anchors.verticalCenter: parent.verticalCenter
      // Note the language is passed to reevaluate this binding on a language change.
      text: Translations.name(root.category, Translations.language)
      font.bold: true
      elide: Text.ElideRight
    }
  }

  MouseArea
  {
    id: mouseArea

    // Properties
    anchors.fill: parent
    hoverEnabled: true

    // Connections
    onClicked: { root.toggled() }
  }
}
