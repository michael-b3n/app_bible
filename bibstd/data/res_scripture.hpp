#pragma once

#include <cstddef>
#include <span>

namespace bibstd::data
{

///
/// Get the number of available zip files.
/// \return number of zip files
///
auto zip_file_count() -> std::size_t;

///
/// Get the raw data of the zip file at given index.
/// \param index Index of the zip file
/// \return span of byte data of the zip file
///
auto zip_file_raw(std::size_t index) -> std::span<const std::byte>;

} // namespace bibstd::data
