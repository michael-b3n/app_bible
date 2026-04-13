import QtQuick
import QtQuick.Controls
import BibQml

Item
{
  id: root

  required property BridgeBibleRefOcr bridgeBibleRefOcr

  Connections
  {
    target: root.bridgeBibleRefOcr
    function onHtmlPassageBeginIndexChanged(index)
    {
      Qt.callLater(textEdit.scrollToIndex, index)
    }
  }

  ScrollView
  {
    id: scrollView
    anchors.fill: parent
    clip: true

    ScrollBar.vertical.policy: ScrollBar.AsNeeded
    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

    TextEdit
    {
      id: textEdit
      width: root.width
      text: root.bridgeBibleRefOcr.htmlPassage
      textFormat: Text.RichText
      wrapMode: Text.WordWrap
      palette.text: Colors.text
      font.pointSize: 12

      readOnly: true
      cursorVisible: false
      selectByMouse: false

      function scrollToIndex(charIndex)
      {
        if(charIndex < 0)
        {
          return
        }
        let rect = positionToRectangle(charIndex)
        let targetY = rect.y - scrollView.height / 2 + rect.height / 2
        targetY = Math.max(0, Math.min(targetY, textEdit.height - scrollView.height))
        scrollView.contentItem.contentY = targetY
      }
    }
  }
}
