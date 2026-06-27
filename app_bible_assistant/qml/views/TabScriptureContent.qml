pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import BibQml

Item
{
  id: root

  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property ScriptureListModel listModelPassage

  // Sticky header showing current book and chapter
  Rectangle
  {
    id: stickyHeader

    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    height: stickyHeaderText.implicitHeight + Metrics.spacingMedium
    color: Colors.backgroundSolid
    z: 1

    Text
    {
      id: stickyHeaderText

      anchors.centerIn: parent
      text: listView.currentBook + (listView.currentChapter > 0 ? " " + listView.currentChapter : "")
      font.pointSize: Metrics.fontSizeBody
      font.bold: true
      color: Colors.text
      renderType: Text.CurveRendering
    }

    Rectangle
    {
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      height: 1
      color: Colors.border
    }
  }

  ListView
  {
    id: listView

    property string currentBook: ""
    property int currentChapter: 0

    anchors.top: stickyHeader.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    clip: true
    model: root.listModelPassage
    spacing: Metrics.spacingTiny
    cacheBuffer: 600

    onContentYChanged: updateHeader()
    onCountChanged: updateHeader()

    function updateHeader()
    {
      if(!model || model.rowCount() === 0)
      {
        currentBook = ""
        currentChapter = 0
        return
      }
      let row = indexAt(contentX, contentY)
      if(row >= 0)
      {
        let idx = model.index(row, 0)
        currentBook = model.data(idx, ScriptureListModel.BookNameRole)
        currentChapter = Number(model.data(idx, ScriptureListModel.ChapterNumberRole))
      }
    }

    function loadPrevious(count)
    {
      if(atYBeginning && model && model.rowCount() > 0)
      {
        let prevCount = model.rowCount()
        model.loadPrevious(count)
        // Maintain scroll position after prepending
        let addedCount = model.rowCount() - prevCount
        if(addedCount > 0)
        {
          positionViewAtIndex(addedCount, ListView.Beginning)
        }
      }
    }

    Connections
    {
      target: root.listModelPassage
      function onRefreshed() { Qt.callLater(function() { listView.loadPrevious(1) }) }
    }

    // Dynamic loading on scroll
    onAtYBeginningChanged:
    {
      loadPrevious(10)
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
      required property int chapterNumber
      required property int verseNumber
      required property bool isHeader

      width: listView.width - vScrollBar.width

      // Book header
      Loader
      {
        active: delegateRoot.isHeader
        width: parent.width
        sourceComponent: Rectangle
        {
          width: parent.width
          height: bookTitle.implicitHeight + Metrics.spacingLarge
          color: Colors.bookHeader
          radius: Metrics.radiusMedium

          Text
          {
            id: bookTitle

            anchors.centerIn: parent
            text: delegateRoot.bookName + " %1".arg(delegateRoot.chapterNumber)
            font.pointSize: Metrics.fontSizeHeading
            font.bold: true
            color: Colors.bookHeaderText
            renderType: Text.CurveRendering
          }
        }
      }

      // Verse row: badge + text
      Row
      {
        width: parent.width
        leftPadding: Metrics.spacingSmall
        spacing: Metrics.spacingMedium

        Rectangle
        {
          id: verseBadge
          width: verseNum.implicitWidth + Metrics.spacingLarge
          height: verseNum.implicitHeight + Metrics.spacingSmall
          radius: Metrics.radiusMedium
          color: Colors.verseBox
          anchors.top: parent.top
          anchors.topMargin: Metrics.spacingTiny

          Text
          {
            id: verseNum

            anchors.centerIn: parent
            text: delegateRoot.verseNumber
            font.pointSize: Metrics.fontSizeSmall
            font.bold: true
            color: Colors.verseText
            renderType: Text.CurveRendering
          }
        }

        Text
        {
          width: parent.width - verseBadge.width - parent.spacing - parent.leftPadding
          text: delegateRoot.verseText
          textFormat: Text.RichText
          wrapMode: Text.WordWrap
          color: Colors.text
          font.pointSize: Metrics.fontSizeBody
          renderType: Text.CurveRendering
        }
      }
    }

    ScrollBar.vertical: ScrollBar
    {
      id: vScrollBar
      policy: ScrollBar.AsNeeded

      background: Rectangle
      {
        implicitWidth: 10
        color: Colors.backgroundSolid
        radius: 5
      }

      contentItem: Rectangle
      {
        implicitWidth: 6
        radius: 3
        color: vScrollBar.pressed ? Colors.pressed : Colors.border
        opacity: vScrollBar.active ? 1.0 : 0.0

        Behavior on opacity { NumberAnimation { duration: 200 } }
      }
    }
  }
}
