#pragma once

#include "bibstd/bible/scripture.hpp"

#include <optional>

// Forward declarations
namespace bibstd::io
{
class zip_file_reader;
} // namespace bibstd::io

namespace bibstd::bible::usx_metadata
{

///
/// Parse the "metadata.xml" descriptor of a USX bundle. Fields missing from
/// the descriptor are substituted by their unknown placeholder, so a successful
/// parse always yields a complete set of information.
/// \return Information about the scripture, or std::nullopt if the descriptor
/// is missing or cannot be parsed
///
auto parse(const io::zip_file_reader& zip_reader) -> std::optional<scripture::info_type>;

} // namespace bibstd::bible::usx_metadata
