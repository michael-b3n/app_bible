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
    LOG_ERROR("formatter \"format_bibleserver_de\" specifies no translations: {}", range);
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

  auto urls = std::vector<std::string>{};
  const auto append_url = [&](const auto book, const auto chapter, const auto verse_begin, const auto verse_end)
  {
    urls.emplace_back(
      std::format(
        "https://www.bibleserver.com/{}/{}{}%2C{}-{}",
        translation_str,
        bible::book_name_variants_de::pretty_names.at(book),
        chapter,
        verse_begin,
        verse_end
      )
    );
  };

  decltype(auto) begin = range.begin();
  decltype(auto) end = range.end();
  if(begin.book() != end.book())
  {
    LOG_WARN("formatter \"format_bibleserver_de\" only supports one book: {}", range);
    append_url(begin.book(), begin.chapter(), begin.verse(), begin.verse());
  }
  else if(begin.chapter() != end.chapter())
  {
    const auto book = begin.book();
    std::ranges::for_each(
      std::views::iota(begin.chapter().value, end.chapter().value + 1),
      [&](const auto chapter_value)
      {
        const auto verse_begin = [&]
        {
          if(chapter_value == begin.chapter().value) return begin.verse();
          return bible::reference::verse_type{1};
        }();
        const auto verse_end = [&]
        {
          if(chapter_value == end.chapter().value) return end.verse();
          return bible::reference::verse_type{bible::verse_count(book, chapter_value).value()};
        }();
        append_url(begin.book(), chapter_value, verse_begin, verse_end);
      }
    );
  }
  else
  {
    append_url(begin.book(), begin.chapter(), begin.verse(), end.verse());
  }
  return std::ranges::all_of(urls, [](const auto& url) { return system::open_browser::open(url); });
}

} // namespace bibstd::core
