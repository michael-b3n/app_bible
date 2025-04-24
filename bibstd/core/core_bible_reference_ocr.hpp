#pragma once

#include "bible/reference_range.hpp"
#include "core/core_bible_reference_ocr_common.hpp"
#include "core/core_tesseract_common.hpp"
#include "math/value_range.hpp"
#include "txt/indexed_strings.hpp"
#include "util/screen_types.hpp"

#include <string_view>

namespace bibstd::core
{

///
/// Core bible reference OCR. This class calls contains functions to parse the OCR data for bible references.
///
class core_bible_reference_ocr final
{
public: // Typedefs
  using screen_rect_type = util::screen_types::screen_rect_type;
  using screen_coordinates_type = util::screen_types::screen_coordinates_type;
  using pixel_plane_type = util::screen_types::pixel_plane_type;
  using tesseract_choice = core_tesseract_common::tesseract_choice;
  using tesseract_choices = core_tesseract_common::tesseract_choices;

public: // Structors
  core_bible_reference_ocr() = default;
  ~core_bible_reference_ocr() noexcept = default;

public: // Operations
  ///
  /// Find bible book given OCR character data. Tesseract choices must be sorted by confidence and must not be empty.
  /// \param choices_list OCR character choices
  /// \param choices_filter Filter function that is called for each choice. If the filter returns false, the choices are ignored
  /// and the main symbol is taken for the result chars.
  /// \return vector of indexed strings with the bible book names inserted at possible positions.
  /// The index count is guaranteed to be the same as the number of choices in the choices_list.
  ///
  [[nodiscard]] auto match_choices_to_bible_book(
    const std::vector<tesseract_choices>& choices_list, const std::function<bool(const tesseract_choices&)>& choices_filter
  ) const -> std::vector<txt::indexed_strings>;

  ///
  /// Generate a list of rectangles that are used to capture the screen for OCR.
  /// \param cursor_position Cursor position on the screen
  /// \param char_height Height of the character, if not given, the screen areas are generated with a default value
  /// \return vector of screen rectangles that are used to capture the screen for OCR
  ///
  [[nodiscard]]
  auto generate_capture_areas(const screen_coordinates_type& cursor_position, std::optional<std::uint32_t> char_height) const
    -> std::vector<screen_rect_type>;

  ///
  /// Check if the given capture area is valid. The capture area is valid if the OCR character data is within the capture area
  /// including a boundary margin.
  /// \param image_dimensions Image dimensions of the captured screen area
  /// \param position_data OCR character position data
  /// \param index_range Index range of the character data that is used to check if the capture area is valid
  /// \return true if the capture area is valid, otherwise false
  ///
  [[nodiscard]] auto is_valid_capture_area(
    const screen_rect_type& image_dimensions,
    const core_bible_reference_ocr_common::reference_position_data& position_data,
    const core_bible_reference_ocr_common::index_range_type& index_range
  ) -> bool;

private: // Constants
  static constexpr auto height_to_width_ratio = 8;
  static constexpr auto vertical_range_to_full_screen_factor = 32;
  static constexpr auto capture_ocr_area_steps = std::array{1, 2, 4};
  static constexpr auto horizontal_margin_multiplier = 2.0;
  static constexpr auto vertical_margin_multiplier = 0.25;

private: // Implementation
  ///
  /// Find text template given OCR character data. Tesseract choices must be sorted by confidence and must not be empty.
  /// \param choices_list OCR character choices
  /// \param text_template Text template that shall be chosen from the character choices
  /// \param choices_filter Filter function that is called for each choice. If the filter returns false, the choices are ignored
  /// and the main symbol is taken for the result chars.
  /// \return vector of indexed strings with the text template symbols inserted at possible positions
  /// The index count is guaranteed to be the same as the number of choices in the choices_list.
  ///
  [[nodiscard]] auto match_choices_to_string(
    const std::vector<tesseract_choices>& choices_list,
    std::string_view text_template,
    const std::function<bool(const tesseract_choices&)>& choices_filter
  ) const -> std::vector<txt::indexed_strings>;

  ///
  /// Check if the given character is found in the choices.
  /// \param choices OCR character choices.
  /// \param chars Expected characters of which the begin is checked to find in the choices
  /// \return optional tesseract choice if the character is found, otherwise nullopt
  ///
  [[nodiscard]] auto find_chars_begin_match(const tesseract_choices& choices, std::string_view chars) const
    -> std::optional<tesseract_choice>;
};

} // namespace bibstd::core
