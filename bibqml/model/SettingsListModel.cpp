#include "bibqml/model/SettingsListModel.hpp"

#include <bibstd/workflow/workflow_settings.hpp>

namespace bibqml
{

///
///
SettingsListModel::SettingsListModel(
  std::shared_ptr<bibstd::workflow::workflow_settings> workflow_settings, const bibstd::util::non_owning_ptr<QObject> parent
)
  : QAbstractListModel{parent}
  , workflow_settings_{std::move(workflow_settings)}
{
}

///
///
SettingsListModel::~SettingsListModel() noexcept = default;

///
///
int SettingsListModel::rowCount(const QModelIndex& parent) const
{
  if(parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(entries_.size());
}

///
///
QVariant SettingsListModel::data(const QModelIndex& index, const int role) const
{
  if(!index.isValid() || index.row() < 0 || static_cast<decltype(entries_.size())>(index.row()) >= entries_.size())
  {
    return {};
  }
  [[maybe_unused]] const auto& entry = entries_.at(static_cast<std::size_t>(index.row()));
  switch(role)
  {
  // case VerseTextRole: return entry.verseText;
  // case BookNameRole: return entry.bookName;
  // case ChapterNumberRole: return entry.chapterNumber;
  // case VerseNumberRole: return entry.verseNumber;
  // case IsHeaderRole: return entry.isHeader;
  default: return {};
  }
}

///
///
QHash<int, QByteArray> SettingsListModel::roleNames() const
{
  return {
    // {    VerseTextRole,     "verseText"},
    // {     BookNameRole,      "bookName"},
    // {ChapterNumberRole, "chapterNumber"},
    // {  VerseNumberRole,   "verseNumber"},
    // {     IsHeaderRole,      "isHeader"},
  };
}

///
///
SettingsListModel::Entry SettingsListModel::makeEntry() const
{
  return {};
}

} // namespace bibqml
