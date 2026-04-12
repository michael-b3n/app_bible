import QtQuick
import QtQuick.Controls

Item
{
  id: root

  required property string htmlPassage

  ScrollView
  {
    anchors.fill: parent
    clip: true

    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    Text
    {
      width: root.width
      text: root.htmlPassage
      textFormat: Text.RichText
      wrapMode: Text.Wrap
      color: Colors.text
    }
  }
}
