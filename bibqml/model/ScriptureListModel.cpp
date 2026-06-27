#include "bibqml/model/ScriptureListModel.hpp"

#include <bibstd/bible/common.hpp>
#include <bibstd/bible/reference_formatter_de.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/numeric_cast.hpp>
#include <bibstd/util/ranges.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

#include <algorithm>
#include <tuple>

namespace bibqml
{
namespace detail
{

///
/// Get the default scripture from the workflow scripture.
/// \return default scripture, or std::nullopt if no scripture could be obtained
///
auto default_scripture(bibstd::workflow::workflow_scripture& workflow_scripture)
  -> std::optional<std::shared_ptr<bibstd::bible::scripture>>
{
  static constexpr auto default_scripture_params = bibstd::workflow::workflow_scripture::scripture_params::value_type{};
  auto scripture = workflow_scripture.scripture(default_scripture_params);
  if(!scripture)
  {
    LOG_WARN("failed to get default scripture");
    return std::nullopt;
  }
  return scripture.value().scripture;
}

} // namespace detail

///
///
ScriptureListModel::ScriptureListModel(
  std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture, const bibstd::util::non_owning_ptr<QObject> parent
)
  : QAbstractListModel{parent}
  , workflow_scripture_{std::move(workflow_scripture)}
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
  const auto book = bibstd::util::to_enum<bibstd::bible::book_id>(bookId.toStdString());
  if(!book)
  {
    LOG_WARN("invalid book id: {}", bookId.toStdString());
    return;
  }
  const auto scripture = detail::default_scripture(*workflow_scripture_);
  if(!scripture)
  {
    LOG_WARN("failed to get default scripture");
    return;
  }

  const auto ref = bibstd::bible::reference::create(*book, chapter, verse, scripture.value()->versification());
  if(!ref)
  {
    LOG_WARN("invalid reference: {} {}, {}", bookId.toStdString(), chapter, verse);
    return;
  }

  beginResetModel();
  entries_.clear();
  entries_.push_back(makeEntry(*ref));
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
  const auto scripture = detail::default_scripture(*workflow_scripture_);
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
    [&, prev = std::optional<decltype(ref)>{}]([[maybe_unused]] auto) mutable
    {
      prev = versification.prev(ref);
      if(prev)
      {
        newEntries.push_back(makeEntry(*prev));
        ref = *prev;
      }
      return prev.has_value();
    }
  );

  const auto insertCount = static_cast<int>(newEntries.size());
  beginInsertRows(QModelIndex(), 0, insertCount - 1);
  std::ranges::for_each(newEntries, [&](auto& entry) { entries_.push_front(std::move(entry)); });
  endInsertRows();
}

///
///
void ScriptureListModel::loadNext(const int count)
{
  if(entries_.empty())
  {
    return;
  }
  const auto scripture = detail::default_scripture(*workflow_scripture_);
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
    [&, next = std::optional<decltype(ref)>{}]([[maybe_unused]] auto) mutable
    {
      next = versification.next(ref);
      if(next)
      {
        newEntries.push_back(makeEntry(*next));
        ref = *next;
      }
      return next.has_value();
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
  auto result = workflow_scripture_->passage(params);
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

  return Entry{
    .ref = ref,
    .verseText = fetchPassage(ref),
    .bookName = bookName,
    .chapterNumber = ref.chapter().value,
    .verseNumber = ref.verse().value,
    .isHeader = ref.verse() == decltype(ref.verse()){1},
  };
}

} // namespace bibqml
