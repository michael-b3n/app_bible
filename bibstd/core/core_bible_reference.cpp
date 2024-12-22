#include "core/core_bible_reference.hpp"
#include "bible/book_name_variants_de.hpp"
#include "util/log.hpp"

#include <ranges>

namespace bibstd::core
{

///
///
auto core_bible_reference::find(const std::string_view text) -> std::vector<bible::reference_range>
{
  return parser_.parse(text);
}

} // namespace bibstd::core
