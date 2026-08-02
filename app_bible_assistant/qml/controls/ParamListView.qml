pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.VectorImage
import BibQml

///
/// Editable list parameter object.
/// For editable param lists a list wrapper type
/// is required for values that are convertible to string.
/// The validator type must be none.
///
ParamBase
{
  id: root

  // Components
  contentItem: Rectangle
  {
    // Properties
    width: root.availableWidth
    implicitHeight: listView.contentHeight + Metrics.controlHeight
    color: "transparent"

    // Components
    ListView
    {
      id: listView

      // Properties
      model: SimpleListModel{}
      interactive: true
      clip: false
      anchors.fill: parent

      // Connections
      Connections
      {
        target: root
        function onValueChanged() { listView.model.replace(root.value) }
      }
      Component.onCompleted: { listView.model.replace(root.value) }

      // Components
      delegate: Row
      {
        id: delegateRoot

        // Properties
        required property int index
        required property var value

        width: listView.width
        spacing: Metrics.spacingTiny

        // Components
        Loader
        {
          // Properties
          sourceComponent: delegateRoot.index === 0 ? plusButton : dragArea

          // Components
          Component
          {
            id: plusButton

            // Components
            Rectangle
            {
              // Properties
              color: "transparent"
              width: Metrics.controlHeight - delegateRoot.spacing
              height: delegateRoot.height

              // Components
              ButtonIconSimple
              {
                // Properties
                svgSource: Icons.addToQueue

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                height: width

                // Connections
                onClicked: listView.addVar()
              }
            }
          }
          Component
          {
            id: dragArea

            // Components
            Rectangle
            {
              // Properties
              color: "transparent"
              width: Metrics.controlHeight - delegateRoot.spacing
              height: delegateRoot.height

              // Components
              VectorImage
              {
                source: Icons.dragHandle
                preferredRendererType: VectorImage.CurveRenderer

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                height: width
              }
            }
          }
        }
        Loader
        {
          // Properties
          sourceComponent:
          {
            // param has list wrapper type
            switch(root.valueType)
            {
            case SettingsListModel.BoolValueType: return paramSwitch
            case SettingsListModel.IntValueType: // [[fallthrough]]
            case SettingsListModel.DoubleValueType: // [[fallthrough]]
            case SettingsListModel.TimeValueType: // [[fallthrough]]
            case SettingsListModel.StringValueType: // [[fallthrough]]
            case SettingsListModel.PathValueType:
              switch(root.validatorType)
              {
              case SettingsListModel.UnboundValidatorType: // [[fallthrough]]
              case SettingsListModel.RangeValidatorType: return paramTextField
              case SettingsListModel.ListValidatorType: return paramComboBox
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
              id: switchControl

              // Properties
              path: root.path
              valueType: root.valueType
              wrapperType: root.wrapperType
              validatorType: root.validatorType
              value: delegateRoot.value
              listValidatorData: root.listValidatorData

              showTitle: false
              width: delegateRoot.width

              // Connections
              onParamValueChanged: (value) => { listView.updateItem(delegateRoot.index, value) }

              // Components
              ButtonRemoveElement
              {
                // Properties
                paramControl: switchControl

                // Connections
                onClicked: listView.removeVar(delegateRoot.index)
              }
            }
          }
          Component
          {
            id: paramTextField

            // Components
            ParamTextField
            {
              id: textFieldControl

              // Properties
              path: root.path
              valueType: root.valueType
              wrapperType: root.wrapperType
              validatorType: root.validatorType
              value: delegateRoot.value
              listValidatorData: root.listValidatorData

              showTitle: false
              width: delegateRoot.width - Metrics.controlHeight

              // Connections
              onParamValueChanged: (value) => { listView.updateItem(delegateRoot.index, value) }

              // Components
              ButtonRemoveElement
              {
                // Properties
                paramControl: textFieldControl

                // Connections
                onClicked: listView.removeVar(delegateRoot.index)
              }
            }
          }
          Component
          {
            id: paramComboBox

            // Components
            ParamComboBox
            {
              id: comboBoxControl

              // Properties
              path: root.path
              valueType: root.valueType
              wrapperType: root.wrapperType
              validatorType: root.validatorType
              value: delegateRoot.value
              listValidatorData: root.listValidatorData

              showTitle: false
              width: delegateRoot.width - Metrics.controlHeight

              // Connections
              onParamValueChanged: (value) => { listView.updateItem(delegateRoot.index, value) }

              // Components
              ButtonRemoveElement
              {
                // Properties
                paramControl: comboBoxControl

                // Connections
                onClicked: listView.removeVar(delegateRoot.index)
              }
            }
          }
          Component
          {
            id: paramError

            // Components
            ParamError
            {
              // Properties
              path: root.path
              valueType: root.valueType
              wrapperType: root.wrapperType
              validatorType: root.validatorType
              value: delegateRoot.value
              listValidatorData: root.listValidatorData

              showTitle: false
              width: delegateRoot.width - Metrics.controlHeight
            }
          }
        }
        DragHandler
        {
          id: dragHandler

          // Properties
          target: null
          xAxis.enabled: false
          grabPermissions: PointerHandler.CanTakeOverFromAnything

          property real startY: 0
          property int startIndex: -1
          property int desiredIndex: -1

          // Connections
          onActiveChanged:
          {
            if (active)
            {
              listView.interactive = false
              startY = delegateRoot.y
              startIndex = delegateRoot.index
              desiredIndex = startIndex
            }
            else
            {
              listView.interactive = true
              delegateRoot.y = startY
              if(desiredIndex !== startIndex)
              {
                listView.moveItem(startIndex, desiredIndex)
              }
            }
          }
          onTranslationChanged:
          {
            if(!active)
            {
              return
            }

            delegateRoot.y = startY + translation.y

            const offset = Math.round(translation.y / delegateRoot.height)
            desiredIndex = Math.max(0, Math.min(listView.count - 1, startIndex + offset))
          }
        }
      }

      // Functions
      function addVar()
      {
        switch(root.valueType)
        {
        case SettingsListModel.BoolValueType:
          listView.model.prepend(false)
          break
        case SettingsListModel.IntValueType:
          listView.model.prepend(0)
          break
        case SettingsListModel.DoubleValueType:
        case SettingsListModel.TimeValueType:
          listView.model.prepend(0.0)
          break
        case SettingsListModel.StringValueType:
        case SettingsListModel.PathValueType:
          listView.model.prepend("")
          break
        default:
          BridgeLogger.error("unsupported SettingsListModel::ValueType value")
          return
        }
        root.paramValueChanged(listView.model.entries())
      }
      function removeVar(index)
      {
        let idx = listView.model.index(index, 0)
        if(listView.model.remove(idx))
        {
          root.paramValueChanged(listView.model.entries())
        }
      }
      function updateItem(index, value)
      {
        let idx = listView.model.index(index, 0)
        if(listView.model.setData(idx, value, SimpleListModel.ValueRole))
        {
          root.paramValueChanged(listView.model.entries())
        }
      }
      function moveItem(fromIndex, toIndex)
      {
        let fromIdx = listView.model.index(fromIndex, 0)
        let toIdx = listView.model.index(toIndex, 0)
        if(listView.model.move(fromIdx, toIdx))
        {
          root.paramValueChanged(listView.model.entries())
        }
      }
    }
  }
}
