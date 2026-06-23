#pragma once

#include "bibstd/data/pixel.hpp"
#include "bibstd/data/plane.hpp"

#include <filesystem>
#include <optional>

namespace bibstd::io
{

///
/// Saves the provided pixel object as a bitmap *.bmp file with the name provided by the parameter.
/// Saves only a subarea of the image if the optional subarea is specified.
/// File extension is not forced but should be `*.bmp`.
/// \return true if successful, false otherwise
///
auto save_as_bitmap(
  const data::plane_view<const data::pixel>& data,
  std::optional<data::plane_view<const data::pixel>::area_type> subarea,
  const std::filesystem::path& path
) -> bool;

} // namespace bibstd::io
