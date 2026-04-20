import QtQuick
import QtQuick.Controls
import BibQml

Item
{
  id: root

  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property AbstractListModelPassage listModelPassage

  ListView
  {
    id: listView

    anchors.fill: parent
    clip: true
    model: root.listModelPassage
    spacing: 2

    // Dynamic loading on scroll
    onAtYBeginningChanged:
    {
      if(atYBeginning && model && model.rowCount() > 0)
      {
        let prevCount = model.rowCount()
        model.loadPrevious(10)
        // Maintain scroll position after prepending
        let addedCount = model.rowCount() - prevCount
        if(addedCount > 0)
        {
          positionViewAtIndex(addedCount, ListView.Beginning)
        }
      }
    }

    onAtYEndChanged:
    {
      if(atYEnd && model && model.rowCount() > 0)
      {
        model.loadNext(10)
      }
    }

    delegate: Column
    {
      id: delegateRoot

      required property int index
      required property string verseText
      required property string bookName
      required property int chapter
      required property int verseNumber
      required property bool isBookHeader
      required property bool isChapterHeader

      width: listView.width

      // Book header
      Loader
      {
        active: delegateRoot.isBookHeader
        width: parent.width
        sourceComponent: Rectangle
        {
          width: parent.width
          height: bookTitle.implicitHeight + 12
          color: Colors.bookHeader
          radius: 4

          Text
          {
            id: bookTitle
            anchors.centerIn: parent
            text: delegateRoot.bookName
            font.pointSize: 13
            font.bold: true
            color: Colors.bookHeaderText
          }
        }
      }

      // Chapter header
      Loader
      {
        active: delegateRoot.isChapterHeader
        width: parent.width
        sourceComponent: Item
        {
          width: parent.width
          height: chapterBadge.height + 6

          Rectangle
          {
            id: chapterBadge
            anchors.left: parent.left
            anchors.leftMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            width: chapterLabel.implicitWidth + 12
            height: chapterLabel.implicitHeight + 6
            radius: 3
            color: Colors.chapterBox

            Text
            {
              id: chapterLabel
              anchors.centerIn: parent
              text: qsTr("Kapitel %1").arg(delegateRoot.chapter)
              font.pointSize: 10
              font.bold: true
              color: Colors.chapterText
            }
          }
        }
      }

      // Verse row: badge + text
      Row
      {
        width: parent.width
        spacing: 6

        Rectangle
        {
          id: verseBadge
          width: verseNum.implicitWidth + 8
          height: verseNum.implicitHeight + 4
          radius: 3
          color: Colors.verseBox
          anchors.top: parent.top
          anchors.topMargin: 2

          Text
          {
            id: verseNum
            anchors.centerIn: parent
            text: delegateRoot.verseNumber
            font.pointSize: 9
            font.bold: true
            color: Colors.verseText
          }
        }

        TextEdit
        {
          width: parent.width - verseBadge.width - parent.spacing
          text: delegateRoot.verseText
          textFormat: Text.RichText
          wrapMode: Text.WordWrap
          palette.text: Colors.text
          font.pointSize: 12
          readOnly: true
          cursorVisible: false
          selectByMouse: false
        }
      }
    }

    ScrollBar.vertical: ScrollBar
    {
      policy: ScrollBar.AsNeeded
    }
  }
}
