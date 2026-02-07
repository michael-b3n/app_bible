#include "bibstd/core/core_scripture_store.hpp"
#include "bibstd/bible/parser_usx.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/res/scripture.hpp"
#include "bibstd/util/log.hpp"

#include <algorithm>
#include <optional>
#include <ranges>
#include <string_view>

namespace bibstd::core
{
namespace detail
{

///
/// Check if the given path is a zip file.
/// \param path Filesystem path to check
/// \return true if the file is a zip file, false otherwise
///
auto is_zip_file(const std::filesystem::path& path) -> bool
{
  return path.extension() == std::string_view(".zip");
}

///
/// Get the supported file type from the given path.
/// \param path Filesystem path to check
/// \return supported file type if recognized, std::nullopt otherwise
///
auto file_type(const std::filesystem::path& path) -> std::optional<core_scripture_store::supported_file_type>
{
  if(is_zip_file(path))
  {
    return core_scripture_store::supported_file_type::zip;
  }
  return std::nullopt;
}

} // namespace detail

///
///
core_scripture_store::core_scripture_store()
{
  static constexpr auto file_count = res::scripture::file_count();
  std::ranges::for_each(
    std::views::iota(decltype(file_count){0}, file_count),
    [&](const auto index)
    {
      if(const auto file_type = detail::file_type(res::scripture::file_name(index)))
      {
        const auto file_data = res::scripture::file_raw(index);
        switch(*file_type)
        {
        case core_scripture_store::supported_file_type::zip: load_usx(io::zip_file_reader(file_data)); break;
        default: LOG_WARN("file type not supported: file_name=\"{}\"", res::scripture::file_name(index)); break;
        }
      }
    }
  );
}

///
///
core_scripture_store::~core_scripture_store() noexcept = default;

///
///
auto core_scripture_store::load_usx(const io::zip_file_reader& zip_reader) -> bool
{
  if(!zip_reader.is_open())
  {
    LOG_ERROR("failed to open zip archive for usx format");
    return false;
  }
  auto reader = std::make_unique<bible::parser_usx>(zip_reader);
  const auto loaded = reader->valid();
  if(loaded)
  {
    scripture_data_.emplace_back(std::move(reader));
  }
  return loaded;
}

} // namespace bibstd::core
