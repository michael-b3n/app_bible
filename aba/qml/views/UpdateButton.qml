pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import BibQml

///
/// Icon button telling that an update is ready. It opens a drop down offering the install, which
/// restarts the application.
///
ButtonIconSimple
{
  id: root

  // Properties
  svgSource: Icons.updateAvailable

  // Signals
  signal updateClicked()

  // Connections
  onClicked: { menu.visible ? menu.close() : menu.open() }

  // Components
  PopupSimple
  {
    id: menu

    // Properties
    // The button sits at the right edge of the window, so the drop down opens towards the left
    x: root.width - menu.width
    y: root.height + Metrics.spacingTiny
    // A press on the button itself toggles the drop down instead of closing and reopening it
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

    // Components
    contentItem: ItemDelegate
    {
      id: item

      // Properties
      padding: Metrics.spacingSmall
      // As wide as the longer of both texts, so the entry keeps its size while its text changes on hover
      implicitWidth: Math.max(availableMetrics.width, nowMetrics.width) + item.leftPadding + item.rightPadding

      // Connections
      onClicked:
      {
        menu.close()
        root.updateClicked()
      }

      // Components
      contentItem: TextSimple
      {
        id: itemText

        // Properties
        // Note the language is passed to reevaluate this binding on a language change.
        text: Translations.name(item.hovered ? "update_now" : "update_available", Translations.language)
        horizontalAlignment: Text.AlignHCenter
      }

      TextMetrics
      {
        id: availableMetrics

        // Properties
        font: itemText.font
        text: Translations.name("update_available", Translations.language)
      }

      TextMetrics
      {
        id: nowMetrics

        // Properties
        font: itemText.font
        text: Translations.name("update_now", Translations.language)
      }

      // Style
      background: BackgroundSimple { color: item.hovered ? Colors.selection : Colors.backgroundSolidDarker }
    }
  }
}
