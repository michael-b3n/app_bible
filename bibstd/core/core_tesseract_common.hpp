#pragma once

#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "math/rect.hpp"
#include "util/const_bimap.hpp"

#include <string_view>

namespace bibstd::core
{

///
/// Core tesseract common. This helper struct contains helper types for core tesseract.
///
struct core_tesseract_common final
{
  // Typedefs
  enum class language
  {
    de,
  };

  using pixel_plane_type = data::plane<data::pixel>;
  using bounding_box_type = math::rect<std::int32_t>;
};

} // namespace bibstd::core
