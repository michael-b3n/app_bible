pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import BibQml

///
/// This objects describes the content of the scripture tab.
/// A list view lists dynamically loaded verses from the listModelScripture.
///
Item
{
  id: root

  // Properties
  required property ScriptureListModel listModelScripture
  required property BridgeBibleRefOcr bridgeBibleRefOcr
  required property BridgeBibleRefLookup bridgeBibleRefLookup

  // Components
  ///
  /// Sticky header showing current book and chapter
  ///
  Rectangle
  {
    id: stickyHeader

    // Properties
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    height: stickyHeaderText.implicitHeight + Metrics.spacingMedium
    color: Colors.backgroundSolid
    z: 1

    // Components
    Text
    {
      id: stickyHeaderText

      // Properties
      anchors.centerIn: parent
      text: listView.currentBook + (listView.currentChapter > 0 ? " " + listView.currentChapter : "")
      font.pointSize: Metrics.fontSizeBody
      font.bold: true
      color: Colors.text
      renderType: Text.CurveRendering
    }

    ///
    /// Opens the chapter currently shown in the header in the browser.
    ///
    ButtonIconSimple
    {
      id: openInBrowserButton

      // Properties
      anchors.right: parent.right
      anchors.rightMargin: Metrics.spacingSmall
      anchors.verticalCenter: stickyHeaderText.verticalCenter
      width: stickyHeaderText.implicitHeight
      height: stickyHeaderText.implicitHeight
      visible: listView.currentChapter > 0
      enabled: openInBrowserButton.visible && !root.bridgeBibleRefLookup.running
      svgSource: Icons.openInBrowser

      // Connections
      onClicked: { root.bridgeBibleRefLookup.lookupChapter(listView.currentBookId, listView.currentChapter) }
    }

    Rectangle
    {
      // Properties
      anchors.bottom: parent.bottom
      anchors.left: parent.left
      anchors.right: parent.right
      height: Metrics.border
      color: Colors.border
    }
  }

  ///
  /// List view with the scripture content.
  ///
  ListView
  {
    id: listView

    // Properties
    // Number of verses that are requested from the model whenever
    // an end of the already loaded scripture range is reached.
    readonly property int pageSize: 10
    // Pixels of delegates kept alive outside the visible area.
    readonly property int cachedPixels: 600

    property string currentBook: ""
    property string currentBookId: ""
    property int currentChapter: 0

    anchors.top: stickyHeader.bottom
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    clip: true
    model: root.listModelScripture
    spacing: Metrics.spacingTiny
    cacheBuffer: listView.cachedPixels

    // Connections
    onContentYChanged: listView.updateHeader()
    onCountChanged: listView.updateHeader()
    onAtYBeginningChanged: listView.loadPrevious(listView.pageSize)
    onAtYEndChanged:
    {
      if(listView.atYEnd && listView.model && listView.model.rowCount() > 0)
      {
        listView.model.loadNext(listView.pageSize)
      }
    }
    Connections
    {
      target: root.listModelScripture
      function onRefreshed() { Qt.callLater(function() { listView.loadPrevious(1) }) }
    }

    // Components
    delegate: Column
    {
      id: delegateRoot

      // Properties
      required property int index
      required property string verseText
      required property string bookName
      required property int chapterNumber
      required property int verseNumber
      required property bool isHeader
      width: listView.width - scrollBar.width

      // Components
      ///
      /// Book header
      ///
      Loader
      {
        // Properties
        active: delegateRoot.isHeader
        width: parent.width
        sourceComponent: Rectangle
        {
          // Properties
          width: parent.width
          height: bookTitle.implicitHeight + Metrics.spacingLarge
          color: Colors.bookHeader
          radius: Metrics.radiusMedium

          // Components
          Text
          {
            id: bookTitle

            // Properties
            anchors.centerIn: parent
            text: delegateRoot.bookName + " %1".arg(delegateRoot.chapterNumber)
            font.pointSize: Metrics.fontSizeHeading
            font.bold: true
            color: Colors.bookHeaderText
            renderType: Text.CurveRendering
          }
        }
      }

      ///
      /// Verse row: badge + text
      ///
      Row
      {
        // Properties
        width: parent.width
        leftPadding: Metrics.spacingSmall
        spacing: Metrics.spacingMedium

        // Components
        ///
        /// Badge with verse number
        ///
        Rectangle
        {
          id: verseBadge

          // Properties
          width: verseNum.implicitWidth + Metrics.spacingLarge
          height: verseNum.implicitHeight + Metrics.spacingSmall
          radius: Metrics.radiusMedium
          color: Colors.verseBox
          anchors.top: parent.top
          anchors.topMargin: Metrics.spacingTiny

          Text
          {
            id: verseNum

            // Properties
            anchors.centerIn: parent
            text: delegateRoot.verseNumber
            font.pointSize: Metrics.fontSizeSmall
            font.bold: true
            color: Colors.verseText
            renderType: Text.CurveRendering
          }
        }

        ///
        /// Scripture text
        ///
        Text
        {
          // Properties
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

    ScrollBar.vertical: ScrollBarSimple{ id: scrollBar }

    // Functions
    function updateHeader()
    {
      if(!listView.model || listView.model.rowCount() === 0)
      {
        listView.currentBook = ""
        listView.currentBookId = ""
        listView.currentChapter = 0
        return
      }
      let row = listView.indexAt(listView.contentX, listView.contentY)
      if(row >= 0)
      {
        let idx = listView.model.index(row, 0)
        listView.currentBook = listView.model.data(idx, ScriptureListModel.BookNameRole)
        listView.currentBookId = listView.model.data(idx, ScriptureListModel.BookIdRole)
        listView.currentChapter = Number(listView.model.data(idx, ScriptureListModel.ChapterNumberRole))
      }
    }

    function loadPrevious(count)
    {
      if(listView.atYBeginning && listView.model && listView.model.rowCount() > 0)
      {
        let prevCount = listView.model.rowCount()
        listView.model.loadPrevious(count)
        // Maintain scroll position after prepending
        let addedCount = listView.model.rowCount() - prevCount
        if(addedCount > 0)
        {
          listView.positionViewAtIndex(addedCount, ListView.Beginning)
        }
      }
    }
  }
}
