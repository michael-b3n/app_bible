#pragma once

#include <bibstd/bible/reference.hpp>
#include <bibstd/util/non_owning_ptr.hpp>

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

  // Typedefs
  ///
  /// ListModel Entry
  ///
  struct Entry final
  {
    bibstd::bible::reference ref;
    QString verseText;
    QString bookName;
    std::uint32_t chapterNumber;
    std::uint32_t verseNumber;
    bool isHeader;
  };

  // Variables
  std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture_;
  std::deque<Entry> entries_;

public: // Typedefs
  enum Role
  {
    VerseTextRole = Qt::UserRole + 1,
    BookNameRole,
    ChapterNumberRole,
    VerseNumberRole,
    IsHeaderRole,
  };
  Q_ENUM(Role)

public: // Structors
  explicit ScriptureListModel(
    std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture,
    bibstd::util::non_owning_ptr<QObject> parent = nullptr
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

private: // Implementation
  QString fetchPassage(const bibstd::bible::reference& ref) const;
  Entry makeEntry(const bibstd::bible::reference& ref) const;
  void addEntry(const bibstd::bible::reference& ref);
};

} // namespace bibqml
