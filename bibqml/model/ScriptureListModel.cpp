#include "bibqml/model/ScriptureListModel.hpp"
#include "bibqml/util/ScriptureAccess.hpp"

#include <bibstd/bible/common.hpp>
#include <bibstd/bible/reference_formatter_de.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/numeric_cast.hpp>
#include <bibstd/util/ranges.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

#include <algorithm>
#include <limits>
#include <tuple>

namespace bibqml
{
namespace detail
{

///
/// Maximum number of entries the model can hold, limited by the row index type of QAbstractListModel.
///
constexpr auto maxEntries = static_cast<std::size_t>(std::numeric_limits<int>::max());

} // namespace detail

///
///
ScriptureListModel::ScriptureListModel(
  std::shared_ptr<bibstd::workflow::workflow_scripture> workflowScripture, const bibstd::util::non_owning_ptr<QObject> parent
)
  : QAbstractListModel{parent}
  , workflowScripture_{std::move(workflowScripture)}
{
}

///
///
ScriptureListModel::~ScriptureListModel() noexcept = default;

///
///
int ScriptureListModel::rowCount(const QModelIndex& parent) const
{
  if(parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(entries_.size());
}

///
///
QVariant ScriptureListModel::data(const QModelIndex& index, const int role) const
{
  if(!index.isValid() || index.row() < 0 || static_cast<decltype(entries_.size())>(index.row()) >= entries_.size())
  {
    return {};
  }
  const auto& entry = entries_.at(static_cast<std::size_t>(index.row()));
  switch(role)
  {
  case VerseTextRole: return entry.verseText;
  case BookIdRole: return entry.bookId;
  case BookNameRole: return entry.bookName;
  case ChapterNumberRole: return entry.chapterNumber;
  case VerseNumberRole: return entry.verseNumber;
  case IsHeaderRole: return entry.isHeader;
  default: return {};
  }
}

///
///
QHash<int, QByteArray> ScriptureListModel::roleNames() const
{
  return {
    {    VerseTextRole,     "verseText"},
    {       BookIdRole,        "bookId"},
    {     BookNameRole,      "bookName"},
    {ChapterNumberRole, "chapterNumber"},
    {  VerseNumberRole,   "verseNumber"},
    {     IsHeaderRole,      "isHeader"},
  };
}

///
///
void ScriptureListModel::resetWithReference(const QString& bookId, const int chapter, const int verse)
{
  const auto ref = toReference(*workflowScripture_, bookId, chapter, verse);
  if(!ref)
  {
    return;
  }

  beginResetModel();
  entries_.clear();
  addEntry(*ref);
  endResetModel();
  // load some context around the initial verse
  loadNext(10);
  emit refreshed();
}

///
///
void ScriptureListModel::loadPrevious(const int count)
{
  if(entries_.empty())
  {
    return;
  }
  const auto scripture = defaultScripture(*workflowScripture_);
  if(!scripture)
  {
    return;
  }
  decltype(auto) versification = scripture.value()->versification();

  auto ref = entries_.front().ref;
  auto newEntries = std::vector<Entry>{};
  newEntries.reserve(static_cast<std::size_t>(count));

  std::ignore = std::ranges::all_of(
    bibstd::util::ranges::index_view_to(count),
    [&]([[maybe_unused]] auto) mutable
    {
      const auto prev = versification.prev(ref);
      if(!prev)
      {
        return false;
      }
      if(entries_.size() + newEntries.size() >= detail::maxEntries)
      {
        LOG_ERROR("max entries count exceeded: reference=\"{}\" not added", *prev);
        return false;
      }
      newEntries.push_back(makeEntry(*prev));
      ref = *prev;
      return true;
    }
  );

  if(!newEntries.empty())
  {
    const auto insertCount = static_cast<int>(newEntries.size());
    beginInsertRows(QModelIndex(), 0, insertCount - 1);
    std::ranges::for_each(newEntries, [&](auto& entry) { entries_.push_front(std::move(entry)); });
    endInsertRows();
  }
}

///
///
void ScriptureListModel::loadNext(const int count)
{
  if(entries_.empty())
  {
    return;
  }
  const auto scripture = defaultScripture(*workflowScripture_);
  if(!scripture)
  {
    return;
  }
  decltype(auto) versification = scripture.value()->versification();

  auto ref = entries_.back().ref;
  auto newEntries = std::vector<Entry>{};
  newEntries.reserve(static_cast<std::size_t>(count));

  std::ignore = std::ranges::all_of(
    bibstd::util::ranges::index_view_to(count),
    [&]([[maybe_unused]] auto) mutable
    {
      const auto next = versification.next(ref);
      if(!next)
      {
        return false;
      }
      if(entries_.size() + newEntries.size() >= detail::maxEntries)
      {
        LOG_ERROR("max entries count exceeded: reference=\"{}\" not added", *next);
        return false;
      }
      newEntries.push_back(makeEntry(*next));
      ref = *next;
      return true;
    }
  );

  if(!newEntries.empty())
  {
    const auto insertCount = static_cast<int>(newEntries.size());
    const auto startRow = static_cast<int>(entries_.size());
    beginInsertRows(QModelIndex(), startRow, startRow + insertCount - 1);
    std::ranges::for_each(newEntries, [&](auto& entry) { entries_.push_back(std::move(entry)); });
    endInsertRows();
  }
}

///
///
void ScriptureListModel::clear()
{
  beginResetModel();
  entries_.clear();
  endResetModel();
}

///
///
QString ScriptureListModel::fetchPassage(const bibstd::bible::reference& ref) const
{
  auto params = bibstd::workflow::workflow_scripture::passage_params::value_type{ref, std::nullopt};
  auto result = workflowScripture_->passage(params);
  if(result)
  {
    return QString::fromStdString(result->passage.content);
  }
  return QString{"..."};
}

///
///
ScriptureListModel::Entry ScriptureListModel::makeEntry(const bibstd::bible::reference& ref) const
{
  const auto& prettyName = bibstd::bible::reference_formatter_de::pretty_names.at(ref.book());
  const auto bookName = QString::fromUtf8(prettyName.data(), static_cast<qsizetype>(prettyName.size()));
  const auto bookId = bibstd::util::enum_name(ref.book());

  return Entry{
    .ref = ref,
    .verseText = fetchPassage(ref),
    .bookId = QString::fromLatin1(bookId.data(), static_cast<qsizetype>(bookId.size())),
    .bookName = bookName,
    .chapterNumber = ref.chapter().value,
    .verseNumber = ref.verse().value,
    .isHeader = ref.verse() == decltype(ref.verse()){1},
  };
}

///
///
void ScriptureListModel::addEntry(const bibstd::bible::reference& ref)
{
  if(entries_.size() >= detail::maxEntries)
  {
    LOG_ERROR("max entries count exceeded: reference=\"{}\" not added", ref);
    return;
  }
  entries_.emplace_back(makeEntry(ref));
}

} // namespace bibqml
