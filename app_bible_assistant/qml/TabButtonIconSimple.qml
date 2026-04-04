import QtQuick
import QtQuick.VectorImage

TabButtonBase
{
  id: root

  required property string svgSource

  VectorImage
  {
    anchors.fill: parent
    source: root.svgSource
    preferredRendererType: VectorImage.CurveRenderer
  }
}
