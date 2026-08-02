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
    implicitHeight: contentText.implicitHeight + 2 * padding

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
      model.replace(root.listValidatorData)
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

    indicator: Canvas
    {
      id: canvas

      // Properties
      contextType: "2d"
      x: control.width - width - Metrics.paddingParamContent
      y: control.topPadding + (control.availableHeight - height) / 2
      width: control.availableHeight / 2
      height: control.availableHeight / 3

      // Connections
      Connections
      {
        target: control
        function onPressedChanged() { canvas.requestPaint(); }
      }
      onPaint:
      {
        context.reset();
        context.moveTo(0, 0);
        context.lineTo(width, 0);
        context.lineTo(width / 2, height);
        context.closePath();
        context.fillStyle = control.pressed ? Colors.pressed : Colors.border;
        context.fill();
      }
    }

    popup: Popup
    {
      // Properties
      y: control.height - 1
      width: control.width
      padding: Metrics.spacingSmall

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
      const selectedValue = root.value
      if(control.model === undefined || control.model === null)
      {
        control.currentIndex = -1
        return
      }

      for(let i = 0; i < control.count; ++i)
      {
        let idx = control.model.index(i, 0)
        const option = control.model.data(idx, SimpleListModel.ValueRole)
        const optionText = option === undefined || option === null ? "" : option.toString()
        const selectedText = selectedValue === undefined || selectedValue === null ? "" : selectedValue.toString()

        if(optionText === selectedText)
        {
          control.currentIndex = i
          return
        }
      }

      control.currentIndex = -1
    }
  }
}

