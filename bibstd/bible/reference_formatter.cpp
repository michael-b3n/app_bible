#include "bibstd/bible/reference_formatter.hpp"
#include "bibstd/bible/book_name_variants_de.hpp"
#include "bibstd/util/const_bimap.hpp"
#include "bibstd/util/log.hpp"

#include <algorithm>
#include <format>
#include <ranges>

namespace bibstd::bible
{

///
///
auto reference_formatter::pretty(const reference_range& range) -> std::string
{
  auto result = std::string{};
  decltype(auto) begin = range.begin();
  decltype(auto) end = range.end();
  if(begin.book() != end.book())
  {
    result = std::format(
      "{} {}, {} - {} {}, {}",
      book_name_variants_de::pretty_names.at(begin.book()),
      begin.chapter(),
      begin.verse(),
      book_name_variants_de::pretty_names.at(end.book()),
      end.chapter(),
      end.verse()
    );
  }
  else if(begin.chapter() != end.chapter())
  {
    result = std::format(
      "{} {}, {} - {}, {}",
      book_name_variants_de::pretty_names.at(begin.book()),
      begin.chapter(),
      begin.verse(),
      end.chapter(),
      end.verse()
    );
  }
  else if(begin.verse() != end.verse())
  {
    result = std::format(
      "{} {}, {} - {}", book_name_variants_de::pretty_names.at(begin.book()), begin.chapter(), begin.verse(), end.verse()
    );
  }
  else
  {
    result = std::format("{} {}, {}", book_name_variants_de::pretty_names.at(begin.book()), begin.chapter(), begin.verse());
  }
  return result;
}

} // namespace bibstd::bible
