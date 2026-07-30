#include "src/pretty_names.hpp"

#include <bibstd/io/csv_reader.hpp>
#include <bibstd/util/exception.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/ranges.hpp>

#include <algorithm>
#include <format>
#include <ranges>

namespace aba
{
namespace
{

// Index of the column holding the keys. All other columns hold pretty names.
constexpr std::size_t key_column_index = 0;

} // anonymous namespace

///
///
pretty_names::pretty_names(const std::span<const std::byte> csv)
{
  const auto reader = bibstd::io::csv_reader{
    csv, bibstd::io::csv_reader::params{.skip_comment_lines = true, .skip_empty_lines = true}
  };
  const auto columns = reader.column_names();
  if(columns.size() <= key_column_index + 1)
  {
    throw bibstd::util::exception(std::format("pretty names document holds no language column: columns={}", columns.size()));
  }
  languages_ = columns | std::views::drop(key_column_index + 1) | std::ranges::to<std::vector>();

  std::ranges::for_each(
    bibstd::util::ranges::index_view_to(reader.row_count()),
    [&](const auto row)
    {
      auto cells = reader.row(row);
      if(cells.size() != columns.size())
      {
        LOG_ERROR("skip pretty names row: unexpected cell count: row={}, cells={}", row, cells.size());
        return;
      }
      auto key = std::move(cells.at(key_column_index));
      if(key.empty())
      {
        LOG_ERROR("skip pretty names row: empty key: row={}", row);
        return;
      }
      auto names = std::move(cells) | std::views::drop(key_column_index + 1) | std::ranges::to<std::vector>();
      const auto [it, inserted] = entries_.try_emplace(std::move(key), std::move(names));
      if(!inserted)
      {
        LOG_ERROR("duplicate pretty names key: row={}, key=\"{}\"", row, it->first);
      }
    }
  );
  LOG_INFO("pretty names loaded: languages={}, keys={}", languages_.size(), entries_.size());
}

///
///
auto pretty_names::languages() const -> const std::vector<std::string>&
{
  return languages_;
}

///
///
auto pretty_names::name(const std::string_view language, const std::string_view key) const -> std::optional<std::string>
{
  const auto language_it = std::ranges::find(languages_, language);
  if(language_it == std::ranges::cend(languages_))
  {
    return std::nullopt;
  }
  const auto entry_it = entries_.find(std::string{key});
  if(entry_it == std::ranges::cend(entries_))
  {
    return std::nullopt;
  }
  const auto index = static_cast<std::size_t>(std::ranges::distance(std::ranges::cbegin(languages_), language_it));
  if(index >= entry_it->second.size() || entry_it->second.at(index).empty())
  {
    return std::nullopt;
  }
  return entry_it->second.at(index);
}

} // namespace aba
