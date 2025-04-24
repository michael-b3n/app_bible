#pragma once

#include "util/screen_types.hpp"

#include <string>
#include <vector>

namespace bibstd::core
{

///
/// Core bible reference OCR common. This helper struct contains helper types for core bible reference OCR.
///
struct core_bible_reference_ocr_common final
{
  // Typedefs
  using index_range_type = math::value_range<std::size_t>;
  using screen_rect_type = util::screen_types::screen_rect_type;

  struct character_data final
  {
    double distance;
    screen_rect_type bounding_box;
  };

  struct reference_position_data final
  {
    std::string text;
    std::vector<character_data> char_data;
    std::size_t index;
  };
};

} // namespace bibstd::core
