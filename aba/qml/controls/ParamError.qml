import QtQuick

///
/// Error marker for unsupported param types.
/// This is for debugging only and should not be visible
///
ParamBase
{
  id: root

  // Components
  contentItem: ParamBackground
  {
    // Properties
    width: root.availableWidth
    height: errorText.contentHeight

    // Components
    ParamText
    {
      id: errorText

      // Properties
      padding: Metrics.paddingParamContent
      text: "Error"
    }
  }
}
