#include "bibqml/AbstractListModelPassage.hpp"
#include "bibstd/util/ranges.hpp"

#include <bibstd/bible/book_name_variants_de.hpp>
#include <bibstd/bible/common.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

#include <algorithm>
#include <tuple>

namespace bibqml
{

///
///
AbstractListModelPassage::AbstractListModelPassage(std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture, QObject* parent)
  : QAbstractListModel(parent)
  , workflow_scripture_{std::move(workflow_scripture)}
{
}

///
///
AbstractListModelPassage::~AbstractListModelPassage() noexcept = default;

///
///
auto AbstractListModelPassage::rowCount(const QModelIndex& parent) const -> int
{
  if(parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(entries_.size());
}

///
///
auto AbstractListModelPassage::data(const QModelIndex& index, const int role) const -> QVariant
{
  if(!index.isValid() || index.row() < 0 || index.row() >= static_cast<int>(entries_.size()))
  {
    return {};
  }
  const auto& entry = entries_.at(static_cast<std::size_t>(index.row()));
  switch(role)
  {
  case VerseTextRole: return entry.verseText;
  case BookNameRole: return entry.bookName;
  case ChapterRole: return entry.chapter;
  case VerseNumberRole: return entry.verse;
  case IsBookHeaderRole: return entry.isBookHeader;
  case IsChapterHeaderRole: return entry.isChapterHeader;
  default: return {};
  }
}

///
///
auto AbstractListModelPassage::roleNames() const -> QHash<int, QByteArray>
{
  return {
    {      VerseTextRole,       "verseText"},
    {       BookNameRole,        "bookName"},
    {        ChapterRole,         "chapter"},
    {    VerseNumberRole,     "verseNumber"},
    {   IsBookHeaderRole,    "isBookHeader"},
    {IsChapterHeaderRole, "isChapterHeader"},
  };
}

///
///
void AbstractListModelPassage::resetWithReference(const QString& bookId, const int chapter, const int verse)
{
  const auto book = bibstd::util::to_enum<bibstd::bible::book_id>(bookId.toStdString());
  if(!book)
  {
    LOG_WARN("invalid book id: {}", bookId.toStdString());
    return;
  }
  const auto ref =
    bibstd::bible::reference::create(*book, static_cast<std::uint32_t>(chapter), static_cast<std::uint32_t>(verse));
  if(!ref)
  {
    LOG_WARN("invalid reference: {} {}, {}", bookId.toStdString(), chapter, verse);
    return;
  }

  beginResetModel();
  entries_.clear();
  entries_.push_back(makeEntry(*ref, false, false));
  endResetModel();

  // load some context around the initial verse
  loadPrevious(10);
  loadNext(10);
}

///
///
void AbstractListModelPassage::loadPrevious(const int count)
{
  if(entries_.empty())
  {
    return;
  }
  auto ref = entries_.front().ref;
  auto newEntries = std::vector<Entry>{};

  std::ignore = std::ranges::all_of(
    bibstd::util::ranges::index_view_to(count),
    [&]([[maybe_unused]] auto)
    {
      const auto prev = ref;
      --ref;
      const auto valid = ref != prev;
      if(valid)
      {
        const bool bookChanged = ref.book() != prev.book();
        const bool chapterChanged = bookChanged || ref.chapter() != prev.chapter();
        newEntries.push_back(makeEntry(ref, bookChanged, chapterChanged));
      }
      return valid;
    }
  );
  // Reverse since we collected them backwards
  std::ranges::reverse(newEntries);

  // Update headers: the first entry in the model after prepend needs header recalculation
  if(!entries_.empty() && !newEntries.empty())
  {
    auto& firstExisting = entries_.front();
    const auto& lastNew = newEntries.back();
    firstExisting.isBookHeader = firstExisting.ref.book() != lastNew.ref.book();
    firstExisting.isChapterHeader = firstExisting.isBookHeader || firstExisting.ref.chapter() != lastNew.ref.chapter();
  }

  const auto insertCount = static_cast<int>(newEntries.size());
  beginInsertRows(QModelIndex(), 0, insertCount - 1);
  std::ranges::for_each(newEntries | std::views::reverse, [&](auto& entry) { entries_.push_front(std::move(entry)); });
  endInsertRows();
}

///
///
void AbstractListModelPassage::loadNext(const int count)
{
  if(entries_.empty())
  {
    return;
  }
  auto ref = entries_.back().ref;
  auto newEntries = std::vector<Entry>{};

  std::ignore = std::ranges::all_of(
    bibstd::util::ranges::index_view_to(count),
    [&]([[maybe_unused]] auto)
    {
      const auto prev = ref;
      ++ref;
      const auto valid = ref != prev;
      if(valid)
      {
        const bool bookChanged = ref.book() != prev.book();
        const bool chapterChanged = bookChanged || ref.chapter() != prev.chapter();
        newEntries.push_back(makeEntry(ref, bookChanged, chapterChanged));
      }
      return valid;
    }
  );

  const auto insertCount = static_cast<int>(newEntries.size());
  const auto startRow = static_cast<int>(entries_.size());
  beginInsertRows(QModelIndex(), startRow, startRow + insertCount - 1);
  std::ranges::for_each(newEntries, [&](auto& entry) { entries_.push_back(std::move(entry)); });
  endInsertRows();
}

///
///
auto AbstractListModelPassage::fetchPassage(const bibstd::bible::reference& ref) -> QString
{
  auto params = bibstd::workflow::workflow_scripture::process_params{
    {ref, std::nullopt}
  };
  auto result = workflow_scripture_->get(params);
  if(result)
  {
    return QString::fromStdString(result->content);
  }
  return QString{"..."};
}

///
///
auto AbstractListModelPassage::makeEntry(const bibstd::bible::reference& ref, const bool forceBookHeader, const bool forceChapterHeader)
  -> Entry
{
  const auto& prettyName = bibstd::bible::book_name_variants_de::pretty_names.at(ref.book());
  const auto bookName = QString::fromUtf8(prettyName.data(), static_cast<qsizetype>(prettyName.size()));

  return Entry{
    .ref = ref,
    .verseText = fetchPassage(ref),
    .bookName = bookName,
    .chapter = ref.chapter().value,
    .verse = ref.verse().value,
    .isBookHeader = forceBookHeader,
    .isChapterHeader = forceChapterHeader,
  };
}

} // namespace bibqml
