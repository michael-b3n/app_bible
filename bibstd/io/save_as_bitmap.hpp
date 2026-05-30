#pragma once

#include "bibstd/data/pixel.hpp"
#include "bibstd/data/plane.hpp"

#include <filesystem>

namespace bibstd::io
{

///
/// Saves the provided pixel object as a bitmap *.bmp file with the name provided by the parameter.
/// File extension is not forced but should be `*.bmp`.
/// \param data Pixels in a plane, that shall be saved as a *.bmp file
/// \param path of the filename to be written as a bmp image
/// \return true if successful, false otherwise
///
auto save_as_bitmap(const data::plane_view<data::pixel>& data, const std::filesystem::path& path) -> bool;

} // namespace bibstd::io
