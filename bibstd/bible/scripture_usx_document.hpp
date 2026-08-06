#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/scripture_usx.hpp"

#include <optional>
#include <string>

namespace bibstd::bible::usx_document
{

///
/// Content of a single parsed USX book document.
///
struct content final
{
  // names of the book, taken from its header paragraphs
  scripture::book_name_type name;
  // verses of the book, keyed by their reference
  scripture_usx::passage_map_type passages;
};

///
/// Parse the USX document of a single book.
/// \return Content of the book, or std::nullopt if the document cannot be parsed
///
auto parse(book_id book, const std::string& usx_content) -> std::optional<content>;

} // namespace bibstd::bible::usx_document
