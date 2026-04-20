#pragma once

#include <bibstd/bible/reference.hpp>

#include <QAbstractListModel>
#include <QObject>
#include <QtQml/qqmlregistration.h>

#include <deque>
#include <memory>

namespace bibstd::workflow
{
// Forward declaration
class workflow_scripture;
} // namespace bibstd::workflow
namespace bibqml
{

///
/// List model providing bible passages for a ListView.
/// Supports dynamic loading of previous/next verses as the user scrolls.
///
class AbstractListModelPassage final : public QAbstractListModel
{
  Q_OBJECT
  QML_ELEMENT

public: // Typedefs
  enum Role
  {
    VerseTextRole = Qt::UserRole + 1,
    BookNameRole,
    ChapterRole,
    VerseNumberRole,
    IsBookHeaderRole,
    IsChapterHeaderRole,
  };

public: // Structors
  explicit AbstractListModelPassage(
    std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture, QObject* parent = nullptr
  );
  ~AbstractListModelPassage() noexcept override;

public: // QAbstractListModel
  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;
  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
  auto roleNames() const -> QHash<int, QByteArray> override;

public: // Modifiers
  ///
  /// Reset the model with a new starting reference.
  /// Clears all existing entries and loads the initial verse.
  /// \param bookId Book identifier
  /// \param chapter Chapter number
  /// \param verse Verse number
  ///
  Q_INVOKABLE void resetWithReference(const QString& bookId, int chapter, int verse);

  ///
  /// Load more verses before the current first entry.
  /// \param count Number of verses to load
  ///
  Q_INVOKABLE void loadPrevious(int count = 1);

  ///
  /// Load more verses after the current last entry.
  /// \param count Number of verses to load
  ///
  Q_INVOKABLE void loadNext(int count = 1);

private: // Implementation
  struct Entry
  {
    bibstd::bible::reference ref;
    QString verseText;
    QString bookName;
    std::uint32_t chapter;
    std::uint32_t verse;
    bool isBookHeader;
    bool isChapterHeader;
  };

  auto fetchPassage(const bibstd::bible::reference& ref) -> QString;
  auto makeEntry(const bibstd::bible::reference& ref, bool forceBookHeader, bool forceChapterHeader) -> Entry;

private: // Variables
  std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture_;
  std::deque<Entry> entries_;
};

} // namespace bibqml
