#include "bibstd/bible/reference_formatter_de.hpp"

#include <format>

namespace bibstd::bible
{

///
///
auto reference_formatter_de::operator()(const reference_range& range) -> std::string
{
  auto result = std::string{};
  decltype(auto) begin = range.begin();
  decltype(auto) end = range.end();
  if(begin.book() != end.book())
  {
    result = std::format(
      "{} {}, {} - {} {}, {}",
      pretty_names.at(begin.book()),
      begin.chapter(),
      begin.verse(),
      pretty_names.at(end.book()),
      end.chapter(),
      end.verse()
    );
  }
  else if(begin.chapter() != end.chapter())
  {
    result = std::format(
      "{} {}, {} - {}, {}", pretty_names.at(begin.book()), begin.chapter(), begin.verse(), end.chapter(), end.verse()
    );
  }
  else if(begin.verse() != end.verse())
  {
    result = std::format("{} {}, {} - {}", pretty_names.at(begin.book()), begin.chapter(), begin.verse(), end.verse());
  }
  else
  {
    result = std::format("{} {}, {}", pretty_names.at(begin.book()), begin.chapter(), begin.verse());
  }
  return result;
}

} // namespace bibstd::bible
