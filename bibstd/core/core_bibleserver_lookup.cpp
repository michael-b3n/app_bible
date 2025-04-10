#include "core/core_bibleserver_lookup.hpp"
#include "bible/book_name_variants_de.hpp"
#include "system/open_browser.hpp"
#include "util/enum.hpp"
#include "util/log.hpp"

namespace bibstd::core
{

///
///
auto core_bibleserver_lookup::open(const bible::reference_range& range, const std::vector<bible::translation>& translations)
  -> bool
{
  if(translations.empty())
  {
    LOG_ERROR(log_channel, "Formatter \"format_bibleserver_de\" specifies no translations: {}", range);
    return false;
  }

  const auto translation_str = [&]
  {
    auto result = std::string{};
    std::ranges::for_each(
      translations | std::views::filter([&](const auto e) { return translations_map_de.contains(e); }) |
        std::views::transform([&](const auto e) { return translations_map_de.at(e); }),
      [&](const auto e)
      {
        result.append(e.data(), e.size());
        result.push_back('.');
      }
    );
    if(!result.empty()) result.pop_back();
    return result;
  }();

  auto url = std::string{};
  decltype(auto) begin = range.begin();
  decltype(auto) end = range.end();
  if(begin.book() != end.book() || begin.chapter() != end.chapter())
  {
    LOG_WARN(log_channel, "Formatter \"format_bibleserver_de\" only supports one chapter: {}", range);
    url = std::format(
      "https://www.bibleserver.com/{}/{}{}%2C{}",
      translation_str,
      bible::book_name_variants_de::pretty_names.at(begin.book()),
      begin.chapter(),
      begin.verse()
    );
  }
  else
  {
    url = std::format(
      "https://www.bibleserver.com/{}/{}{}%2C{}-{}",
      translation_str,
      bible::book_name_variants_de::pretty_names.at(begin.book()),
      begin.chapter(),
      begin.verse(),
      end.verse()
    );
  }
  return system::open_browser::open(url);
}

} // namespace bibstd::core
