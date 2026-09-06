#include "bibstd/core/core_scripture_store.hpp"
#include "bibstd/bible/scripture_usx.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/res/scripture.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/ranges.hpp"
#include "bibstd/util/string.hpp"

#include <algorithm>
#include <format>
#include <optional>
#include <ranges>
#include <string_view>

namespace bibstd::core
{
namespace
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

} // namespace

///
///
core_scripture_store::core_scripture_store()
{
  static constexpr auto file_count = res::scripture::file_count();
  std::ranges::for_each(
    util::ranges::index_view_to(file_count),
    [&](const auto index)
    {
      const auto file_name = res::scripture::file_name(index);
      LOG_INFO("loading scripture data: file_name=\"{}\"", file_name.stem().string());
      if(const auto type = file_type(file_name))
      {
        const auto file_data = res::scripture::file_raw(index);
        switch(*type)
        {
        case core_scripture_store::supported_file_type::zip: load_usx(io::zip_file_reader(file_data)); return;
        }
      }
      LOG_WARN("file type not supported: file_name=\"{}\"", file_name.stem().string());
    }
  );
}

///
///
core_scripture_store::~core_scripture_store() noexcept = default;

///
///
auto core_scripture_store::scriptures() const -> const scripture_map_type&
{
  return scripture_data_;
}

///
///
auto core_scripture_store::load_usx(const io::zip_file_reader& zip_reader) -> bool
{
  if(!zip_reader.is_open())
  {
    LOG_ERROR("failed to open zip archive for usx format");
    return false;
  }
  auto reader = bible::scripture_usx::create(zip_reader);
  if(reader)
  {
    static constexpr auto uint_ending_format = " ({})";
    auto name = reader->information().name;
    if(scripture_data_.contains(name))
    {
      auto found_max_uint_ending = std::uint32_t{0};
      std::ranges::for_each(
        scripture_data_ | std::views::keys |
          std::views::filter([&](const auto& n) { return util::string::starts_with(name, n); }) |
          std::views::transform([](const auto& n)
                                { return util::string::ends_with_formatted_uint(n, uint_ending_format).value_or(0); }),
        [&](const auto i) { found_max_uint_ending = std::max(found_max_uint_ending, i); }
      );
      name = name + std::format(uint_ending_format, found_max_uint_ending + 1);
    }
    scripture_data_.emplace(name, std::move(reader));
  }
  return reader != nullptr;
}

} // namespace bibstd::core
