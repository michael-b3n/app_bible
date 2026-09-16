import QtQuick
import QtQuick.VectorImage

///
/// Icon button showing a single svg icon.
///
ButtonBase
{
  id: root

  // Properties
  required property string svgSource

  // Components
  VectorImage
  {
    // Properties
    anchors.fill: parent
    source: root.svgSource
    preferredRendererType: VectorImage.CurveRenderer
  }
}
