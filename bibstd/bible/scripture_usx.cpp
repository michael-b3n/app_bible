#include "bibstd/bible/scripture_usx.hpp"
#include "bibstd/bible/common.hpp"
#include "bibstd/bible/scripture_usx_document.hpp"
#include "bibstd/bible/scripture_usx_metadata.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/util/enum.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/timer.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace bibstd::bible
{
namespace
{

///
/// Names and passages of every book of a scripture.
///
struct scripture_content final
{
  scripture_usx::book_name_map_type book_names;
  scripture_usx::passage_map_type passages;
};

///
/// Load a specific entry from the zip reader as a string.
/// \return The loaded entry content as a string, or std::nullopt if not found
///
auto load_entry(const io::zip_file_reader& zip_reader, const std::string& entry_name) -> std::optional<std::string>
{
  using query_flag = io::zip_file_reader::query_flag;
  const auto data = zip_reader.entry(entry_name, {query_flag::exclude_directories, query_flag::case_insensitive});
  if(!data)
  {
    LOG_ERROR("failed to load entry: expected \"{}\" file within archive", entry_name);
    return std::nullopt;
  }
  return zip_reader.read_entry_as_string(*data);
}

///
/// Load and parse the USX document of every book of the scripture.
/// \return Names and passages of all books, or std::nullopt if any book is missing or cannot be parsed
///
auto load_content(const io::zip_file_reader& zip_reader) -> std::optional<scripture_content>
{
  try
  {
    auto result = scripture_content{};
    for(const auto& [id, abbreviation] : scripture_usx::books)
    {
      const auto usx_content = load_entry(zip_reader, std::format("{}.usx", abbreviation));
      if(!usx_content.has_value() || usx_content->empty())
      {
        LOG_ERROR("failed to load \"{}\" data: expected \"{}.usx\" file within archive", util::enum_name(id), abbreviation);
        return std::nullopt;
      }
      auto document = usx_document::parse(id, *usx_content);
      if(!document)
      {
        return std::nullopt;
      }
      result.book_names.emplace(id, std::move(document->name));
      result.passages.merge(document->passages);
    }
    return result;
  }
  catch(...)
  {
    LOG_ERROR("exception while loading book data: {}", util::exception_report());
    return std::nullopt;
  }
}

} // namespace

///
///
auto scripture_usx::create(const io::zip_file_reader& zip_reader) -> std::unique_ptr<scripture>
{
  SCOPED_TIMER_LOG();
  auto info_data = usx_metadata::parse(zip_reader);
  if(!info_data)
  {
    LOG_ERROR("failed to load scripture information data");
    return nullptr;
  }
  auto content = load_content(zip_reader);
  if(!content)
  {
    LOG_ERROR("failed to load scripture book data");
    return nullptr;
  }
  LOG_INFO(
    "loaded scripture: name=\"{}\", abbreviation=\"{}\", language=\"{}\", copyright=\"{}\", verses={}, named_books={}",
    info_data->name,
    info_data->abbreviation,
    info_data->language,
    info_data->copyright.value_or("not found"),
    content->passages.size(),
    content->book_names.size()
  );
  return std::make_unique<scripture_usx>(std::move(*info_data), std::move(content->book_names), std::move(content->passages));
}

///
///
scripture_usx::scripture_usx(info_type info_data, book_name_map_type book_name_data, passage_map_type verse_data)
  : info_data_{std::move(info_data)}
  , book_name_data_{std::move(book_name_data)}
  , verse_data_{std::move(verse_data)}
  , versification_{[&]
                   {
                     auto view = verse_data_ | std::views::keys;
                     const auto v = versification_type{info_data_.name, view | std::ranges::to<std::vector>()};
                     const auto* const it = std::ranges::find(versifications_default, v);
                     return it != std::ranges::cend(versifications_default) ? *it : v;
                   }()}
{
}

///
///
scripture_usx::~scripture_usx() noexcept = default;

///
///
auto scripture_usx::do_information() const -> info_type
{
  return info_data_;
}

///
///
auto scripture_usx::do_book_information(const book_id book) const -> std::optional<book_name_type>
{
  const auto it = book_name_data_.find(book);
  return it != std::cend(book_name_data_) ? std::make_optional(it->second) : std::nullopt;
}

///
///
auto scripture_usx::do_passage_html(const reference_type& ref) const -> std::optional<passage_html_type>
{
  const auto it = verse_data_.find(ref);
  if(it != std::cend(verse_data_))
  {
    return it->second;
  }
  LOG_ERROR("verse not found: {}", ref);
  return std::nullopt;
}

///
///
auto scripture_usx::do_versification() const -> const versification_type&
{
  return versification_;
}

} // namespace bibstd::bible
