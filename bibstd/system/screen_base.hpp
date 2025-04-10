#pragma once

#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "math/rect.hpp"

#include <string_view>

namespace bibstd::system
{

///
/// Screen common definitions.
///
struct screen_base
{
  // Typedefs
  using screen_rect_type = math::rect<std::int32_t>;
  using screen_coordinates_type = screen_rect_type::coordinates_type;
  using pixel_plane_type = data::plane<data::pixel>;

  // Constants
  static constexpr std::string_view log_channel = "system_screen";
};

} // namespace bibstd::system
