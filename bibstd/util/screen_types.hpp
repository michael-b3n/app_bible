#pragma once

#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "math/rect.hpp"

namespace bibstd::util
{

///
/// This class contains common definitions for screen related code.
///
struct screen_types final
{
  // Typedefs
  using screen_rect_type = math::rect<std::int32_t>;
  using screen_coordinates_type = screen_rect_type::coordinates_type;
  using pixel_plane_type = data::plane<data::pixel>;
};

} // namespace bibstd::util
