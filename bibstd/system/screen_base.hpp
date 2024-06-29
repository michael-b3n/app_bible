#pragma once

#include "math/rect.hpp"

namespace bibstd::system
{

///
/// Screen common definitions.
///
struct screen_base
{
  // Typedefs
  using screen_rect_type = math::rect<int>;
  using screen_coordinates_type = screen_rect_type::coordinates_type;
};

} // namespace bibstd::system
