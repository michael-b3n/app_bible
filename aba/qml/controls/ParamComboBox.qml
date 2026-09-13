pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

///
/// Combobox parameter object.
/// For combobox params a valid options list (model)
/// is needed and values must be convertible to string.
///
ParamBase
{
  id: root

  // Components
  contentItem: ComboBox
  {
    id: control

    // Properties
    model: SimpleListModel{}

    // Note the language is passed to reevaluate this binding on a language change.
    displayText: control.currentIndex < 0 ? "" : Translations.name(control.currentText, Translations.language)

    padding: Metrics.paddingParamContent
    spacing: Metrics.spacingTiny
    width: root.availableWidth
    implicitHeight: contentText.implicitHeight + 2 * control.padding

    // Connections
    onActivated: (index) =>
    {
      const idx = control.model.index(index, 0)
      const v = control.model.data(idx, SimpleListModel.ValueRole)
      if(v === undefined)
      {
        control.currentIndex = -1
      }
      else
      {
        root.paramValueChanged(v)
      }
    }
    Connections
    {
      target: root
      function onListValidatorDataChanged()
      {
        control.model.replace(root.listValidatorData)
        Qt.callLater(control.syncCurrentIndex)
      }
      function onValueChanged() { control.syncCurrentIndex() }
    }
    Component.onCompleted:
    {
      control.model.replace(root.listValidatorData)
      Qt.callLater(control.syncCurrentIndex)
    }

    // Components
    contentItem: ParamText
    {
      id: contentText

      // Properties
      text: control.displayText
      elide: Text.ElideRight
      rightPadding: Metrics.paddingParamContent * 2
      // width and height properties of combobox
      // content item is ignored.
    }

    ///
    /// Drop down indicator. It points down while the list is closed and flips over while it is
    /// open, telling that clicking again closes it.
    ///
    indicator: TriangleShape
    {
      // Properties
      x: control.width - width - Metrics.paddingParamContent
      y: control.topPadding + (control.availableHeight - height) / 2
      width: control.availableHeight / 2
      height: control.availableHeight / 3
      color: control.pressed ? Colors.pressed : Colors.border
      transformOrigin: Item.Center
      // Note the popup is null until the control is built, the indicator is declared before it
      rotation: control.popup && control.popup.visible ? 180 : 0

      // Animations
      Behavior on rotation
      {
        NumberAnimation
        {
          duration: Metrics.durationShort
          easing.type: Easing.InOutQuad
        }
      }
      Behavior on color { ColorAnimation { duration: Metrics.durationShort } }
    }

    popup: Popup
    {
      // Properties
      y: control.height - 1
      width: control.width
      padding: Metrics.spacingSmall

      // Animations
      enter: Transition
      {
        NumberAnimation { property: "opacity"; from: 0; to: 1; duration: Metrics.durationShort }
      }
      exit: Transition
      {
        NumberAnimation { property: "opacity"; from: 1; to: 0; duration: Metrics.durationShort }
      }

      // Components
      contentItem: ListView
      {
        id: comboBoxListView

        // Properties
        clip: true
        implicitHeight: contentHeight
        model: control.popup.visible ? control.delegateModel : null
        currentIndex: control.highlightedIndex

        // Components
        ScrollBar.vertical: ScrollBarSimple {}
      }

      // Style
      background: ParamBackground {}
    }

    delegate: ItemDelegate
    {
      id: itemDelegate

      // Properties
      required property var modelData
      required property int index
      highlighted: control.highlightedIndex === itemDelegate.index

      width: comboBoxListView.width
      padding: Metrics.spacingSmall

      // Components
      contentItem: ParamText
      {
        id: textContent

        // Properties
        // Note the language is passed to reevaluate this binding on a language change.
        text:
        {
          const valid = itemDelegate.modelData !== undefined && itemDelegate.modelData !== null
          return valid ? Translations.name(String(itemDelegate.modelData), Translations.language) : ""
        }

        elide: Text.ElideRight
        width: itemDelegate.width
      }

      // Style
      background: ParamBackground
      {
        color: itemDelegate.highlighted ? Colors.selection : Colors.backgroundSolidDarker
        width: itemDelegate.width
      }
    }

    // Style
    background: ParamBackground { color: control.activeFocus ? Colors.selection : Colors.backgroundSolidDarker }

    // Functions
    ///
    /// Sync combobox index with the root value.
    /// This is needed since a value is provided and not an index.
    ///
    function syncCurrentIndex()
    {
      control.currentIndex = control.model ? control.model.indexOfValue(root.value) : -1
    }
  }
}

