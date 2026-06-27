#pragma once

#include <bibstd/bible/reference.hpp>

#include <QAbstractListModel>
#include <QMetaEnum>
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
class ScriptureListModel final : public QAbstractListModel
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
  Q_ENUM(Role)

public: // Structors
  explicit ScriptureListModel(
    std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture, QObject* parent = nullptr
  );
  ~ScriptureListModel() noexcept override;

public: // Overrides
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

public: // Modifiers
  ///
  /// Reset the model with a new starting reference.
  /// Clears all existing entries and loads the initial verse.
  ///
  Q_INVOKABLE void resetWithReference(const QString& bookId, int chapter, int verse);

  ///
  /// Load more verses before the current first entry.
  ///
  Q_INVOKABLE void loadPrevious(int count);

  ///
  /// Load more verses after the current last entry.
  ///
  Q_INVOKABLE void loadNext(int count);

  ///
  /// Clear all entries from the model.
  ///
  Q_INVOKABLE void clear();

signals:
  void refreshed();

private: // Typedefs
  struct Entry final
  {
    bibstd::bible::reference ref;
    QString verseText;
    QString bookName;
    std::uint32_t chapter;
    std::uint32_t verse;
    bool isBookHeader;
    bool isChapterHeader;
  };

private: // Constants
  static constexpr int max_entries_{200};

private: // Implementation
  auto fetchPassage(const bibstd::bible::reference& ref) -> QString;
  auto makeEntry(const bibstd::bible::reference& ref) -> Entry;

private: // Variables
  std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture_;
  std::deque<Entry> entries_;
};

} // namespace bibqml
