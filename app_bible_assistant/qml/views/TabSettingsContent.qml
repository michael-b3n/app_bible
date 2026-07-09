pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import BibQml

///
/// This objects describes the content of the settings tab.
/// A list view lists all settings from the listModelSetting
/// using the ParamsControls.
///
Item
{
  id: root

  // Properties
  required property SettingsListModel listModelSettings

  // Components
  ListView
  {
    id: listView

    // Properties
    anchors.fill: parent
    anchors.margins: Metrics.spacingSmall
    clip: true
    spacing: Metrics.spacingTiny
    model: root.listModelSettings

    // Components
    delegate: Column
    {
      id: delegateRoot

      // Properties
      required property int index
      required property string path
      required property int valueType
      required property int wrapperType
      required property int validatorType
      required property var value
      required property var listValidatorData

      width: listView.width - scrollBar.width

      // Components
      Row
      {
        // Properties
        width: delegateRoot.width
        leftPadding: Metrics.spacingSmall
        spacing: Metrics.spacingLarge

        // Components
        Loader
        {
          // Properties
          sourceComponent:
          {
            switch(delegateRoot.valueType)
            {
            case SettingsListModel.BoolValueType:
            {
              switch(delegateRoot.wrapperType)
              {
              case SettingsListModel.NoneWrapperType: // [[fallthrough]]
              case SettingsListModel.OptionalWrapperType: return paramSwitch
              case SettingsListModel.ListWrapperType: return paramError // unsupported
              default: return paramError
              }
            }
            case SettingsListModel.IntValueType: // [[fallthrough]]
            case SettingsListModel.DoubleValueType: // [[fallthrough]]
            case SettingsListModel.TimeValueType: // [[fallthrough]]
            case SettingsListModel.StringValueType: // [[fallthrough]]
            case SettingsListModel.PathValueType:
              switch(delegateRoot.wrapperType)
              {
              case SettingsListModel.NoneWrapperType: // [[fallthrough]]
              case SettingsListModel.OptionalWrapperType:
                switch(delegateRoot.validatorType)
                {
                case SettingsListModel.UnboundValidatorType: // [[fallthrough]]
                case SettingsListModel.RangeValidatorType: return paramTextField
                case SettingsListModel.ListValidatorType: return paramComboBox
                default: return paramError
                }
              case SettingsListModel.ListWrapperType: return paramListView
              default: return paramError
              }
            default: return paramError
            }
          }

          // Components
          Component
          {
            id: paramSwitch

            // Components
            ParamSwitch
            {
              // Properties
              path: delegateRoot.path
              valueType: delegateRoot.valueType
              wrapperType: delegateRoot.wrapperType
              validatorType: delegateRoot.validatorType
              value: delegateRoot.value
              listValidatorData: delegateRoot.listValidatorData
              width: delegateRoot.width

              // Connections
              onParamValueChanged: (value) => { delegateRoot.writeBack(value) }
            }
          }

          Component
          {
            id: paramTextField

            // Components
            ParamTextField
            {
              // Properties
              path: delegateRoot.path
              valueType: delegateRoot.valueType
              wrapperType: delegateRoot.wrapperType
              validatorType: delegateRoot.validatorType
              value: delegateRoot.value
              listValidatorData: delegateRoot.listValidatorData
              width: delegateRoot.width

              // Connections
              onParamValueChanged: (value) => { delegateRoot.writeBack(value) }
            }
          }

          Component
          {
            id: paramComboBox

            // Components
            ParamComboBox
            {
              // Properties
              path: delegateRoot.path
              valueType: delegateRoot.valueType
              wrapperType: delegateRoot.wrapperType
              validatorType: delegateRoot.validatorType
              value: delegateRoot.value
              listValidatorData: delegateRoot.listValidatorData
              width: delegateRoot.width

              // Connections
              onParamValueChanged: (value) => { delegateRoot.writeBack(value) }
            }
          }

          Component
          {
            id: paramListView

            // Components
            ParamListView
            {
              // Properties
              path: delegateRoot.path
              valueType: delegateRoot.valueType
              wrapperType: delegateRoot.wrapperType
              validatorType: delegateRoot.validatorType
              value: delegateRoot.value
              listValidatorData: delegateRoot.listValidatorData
              width: delegateRoot.width

              // Connections
              onParamValueChanged: (value) => { delegateRoot.writeBack(value) }
            }
          }

          Component
          {
            id: paramError

            // Components
            ParamError
            {
              // Properties
              path: delegateRoot.path
              valueType: delegateRoot.valueType
              wrapperType: delegateRoot.wrapperType
              validatorType: delegateRoot.validatorType
              value: delegateRoot.value
              listValidatorData: delegateRoot.listValidatorData
              width: delegateRoot.width

              // Connections
              onParamValueChanged: (value) => { delegateRoot.writeBack(value) }
            }
          }
        }
      }

      // Functions
      function writeBack(value: var)
      {
        const modelIndex = listView.model.index(delegateRoot.index, 0)
        listView.model.setData(modelIndex, value, SettingsListModel.ValueRole)
      }
    }

    ScrollBar.vertical: ScrollBarSimple { id: scrollBar }
  }
}
