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

  ///
  /// This struct contains OCR data for recognized symbol (character).
  /// \param distance Distance information to a implementation defined reference position
  /// \param bounding_box Bounding box of the recognized symbol
  ///
  struct character_data final
  {
    double distance;
    screen_rect_type bounding_box;
  };

  ///
  /// This struct contains OCR data for recognized reference position.
  /// \param text Text of the recognized area
  /// \param char_data Character data of the recognized area
  /// \param index Index belonging to the character data closest to the reference position
  ///
  struct reference_position_data final
  {
    std::string text;
    std::vector<character_data> char_data;
    std::size_t index;
  };
};

} // namespace bibstd::core
