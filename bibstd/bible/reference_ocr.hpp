#pragma once

#include "bibstd/txt/ocr_engine.hpp"
#include "bibstd/util/screen_types.hpp"

#include <expected>

namespace bibstd::bible
{

///
/// This class initializes ocr engines and provides a set of algorithms
/// to find a bible reference within a provided image.
///
struct reference_ocr final
{
  // Typedefs
  using ocr_engine_list_type = std::vector<txt::ocr_engine_uptr_variant_type>;
  using pixel_plane_view_type = util::pixel_plane_view_type;
  using position_type = util::screen_coordinates_type;

  ///
  /// This struct contains OCR data for recognized reference position:
  /// Text of the relevant part of the recognized area and an index
  /// belonging to the character data closest to the reference position.
  ///
  struct reference_position_data final
  {
    std::string text;
    std::size_t cursor_character_index;
  };

  ///
  /// Enum containing all unexpected OCR results.
  ///
  enum class unexpected_ocr_result
  {
    empty,
    error,
    unsupported
  };

  ///
  /// Enum containing flags for OCR algorithm specification.
  ///
  enum class algorithm_type
  {
    recognize_with_paragraph_recognition,
    recognize_just_with_line_recognition,
  };

  ///
  /// OCR algorithm and result recognition data.
  /// Algorithms specify the algorithms that should run.
  /// They are run in order and used as fallbacks if the results are empty.
  ///
  struct algorithm_data final
  {
    algorithm_type algorithm;
    std::string engine_name_character_recognition;
    std::optional<std::string> engine_name_layout_recognition;
  };

  // Operations
  ///
  /// Run OCR recognition and algorithms on the provided image to find a bible reference at a certain position.
  /// \param engines OCR engines available
  /// \param image Image that shall be recognized
  /// \param position Position where to look for the reference
  /// \param algorithms Flags and data used for the algorithm that shall be run
  /// \return reference position data or unexpected result value
  ///
  static auto run(
    const ocr_engine_list_type& engines,
    const pixel_plane_view_type& image,
    position_type position,
    const algorithm_data& algorithm
  ) -> std::expected<reference_position_data, unexpected_ocr_result>;
};

} // namespace bibstd::bible
