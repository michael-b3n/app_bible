#pragma once

#include "bible/reference_range.hpp"
#include "core/core_bible_reference_ocr_common.hpp"
#include "core/core_tesseract_common.hpp"
#include "math/value_range.hpp"
#include "txt/indexed_strings.hpp"
#include "util/screen_types.hpp"

#include <memory>
#include <optional>
#include <string_view>

namespace bibstd::core
{
// Forward declarations
class core_tesseract;

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
  using character_data = core_bible_reference_ocr_common::character_data;
  using reference_position_data = core_bible_reference_ocr_common::reference_position_data;

  ///
  /// Represents the result of a validity check for a screen capture area.
  /// This struct contains information about whether a screen capture area is valid
  /// and optionally includes the detected character height within the area.
  ///
  struct capture_area_validity_check_result final
  {
    bool valid{false};
    std::optional<std::uint16_t> detected_char_height{};
  };

public: // Structors
  core_bible_reference_ocr(core_tesseract_common::language language);
  ~core_bible_reference_ocr() noexcept;

public: // Operations
  ///
  /// Generate a list of rectangles that are used to capture the screen for OCR.
  /// \param cursor_position Cursor position on the screen
  /// \param char_height Height of the character, if not given, the screen areas are generated with a default value
  /// \return vector of screen rectangles that are used to capture the screen for OCR
  ///
  [[nodiscard]]
  auto generate_capture_areas(const screen_coordinates_type& cursor_position, std::uint16_t assumed_char_height) const
    -> std::vector<screen_rect_type>;

  ///
  /// Capture an area of the screen and start OCR recognition.
  /// \param screen_area Area of the screen that shall be captured
  /// \return true if capturing and recognition was successful, false otherwise
  ///
  [[nodiscard]] auto capture_and_recognize_area(const screen_rect_type& screen_area) const -> bool;

  ///
  /// Find the bounding box of the paragraph containing the given cursor position.
  /// If no paragraph is found at the specified position, returns std::nullopt.
  /// \param relative_cursor_position The position of the cursor on the screen in the image.
  /// \return An optional screen rectangle representing the bounding box of the paragraph.
  /// If no paragraph is found, returns std::nullopt.
  ///
  [[nodiscard]] auto find_paragraph_bounding_box(const screen_coordinates_type& relative_cursor_position) const
    -> std::optional<screen_rect_type>;

  ///
  /// Finds the main reference position data based on the given cursor position.
  /// This function takes the screen coordinates of a cursor position and attempts to
  /// determine the corresponding reference position data. If a valid reference position
  /// is found, it returns the data wrapped in a std::optional. Otherwise, it returns
  /// an empty std::optional.
  /// \param relative_cursor_position The screen coordinates of the cursor position in the image.
  /// \return std::optional<reference_position_data> The reference position data if found,
  /// otherwise an empty std::optional.
  ///
  auto find_main_reference_position_data(const screen_coordinates_type& relative_cursor_position) const
    -> std::optional<reference_position_data>;

  ///
  /// Finds reference position data based on the given cursor position.
  /// This function analyzes the provided cursor position and determines the
  /// corresponding reference position data from a set of OCR choices. The result
  /// is a list of possible `reference_position_data` objects.
  /// \param relative_cursor_position The screen coordinates of the cursor position in the image.
  /// \return A vector containing the reference position data associated with the given cursor position.
  ///
  auto find_reference_position_data_from_choices(const screen_coordinates_type& relative_cursor_position) const
    -> std::vector<reference_position_data>;

  ///
  /// Check if the given capture area is valid. The capture area is valid if the OCR character data is within the capture area
  /// including a boundary margin. If the index range is empty, paragraph left and right borders including the previous and
  /// next text line must be included, such that the capture area is valid.
  /// \param relative_cursor_position The screen coordinates of the cursor position in the image.
  /// \param image_dimensions Image dimensions of the captured screen area
  /// \param paragraph_dimensions Paragraph dimensions within the captured screen area
  /// \param position_data OCR character position data
  /// \param index_range Index range of the character data that is used to check if the capture area is valid
  /// \return true if the capture area is valid, otherwise false
  ///
  [[nodiscard]] auto is_valid_capture_area(
    const screen_coordinates_type& relative_cursor_position,
    const screen_rect_type& image_dimensions,
    const screen_rect_type& paragraph_dimensions,
    const reference_position_data& position_data,
    const core_bible_reference_ocr_common::index_range_type& index_range
  ) -> capture_area_validity_check_result;

private: // Typedefs
  struct line_position_data final
  {
    std::vector<screen_rect_type> line_bounding_boxes;
    std::size_t cursor_line_index;
  };

private: // Constants
  static constexpr auto char_height_multiplier = 4;
  static constexpr auto height_to_width_ratio = 9;
  static constexpr auto capture_ocr_area_steps = std::array{1.0, 1.5, 2.0, 3.0, 4.0};
  static constexpr auto horizontal_margin_multiplier = 2.0;
  static constexpr auto vertical_margin_multiplier = 0.1;

private: // Implementation
  ///
  /// Find bible book given OCR character data. Tesseract choices must be sorted by confidence and must not be empty.
  /// \param choices_list OCR character choices
  /// \param choices_filter Filter function that is called for each choice. If the filter returns false, the choices are
  /// ignored and the main symbol is taken for the result chars.
  /// \return vector of indexed strings with the bible book names inserted at possible positions.
  /// The index count is guaranteed to be the same as the number of choices in the choices_list.
  ///
  [[nodiscard]] auto match_choices_to_bible_book(
    const std::vector<tesseract_choices>& choices_list, const std::function<bool(const tesseract_choices&)>& choices_filter
  ) const -> std::vector<txt::indexed_strings>;

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

  ///
  /// Finds the index of the character data with the minimum distance.
  /// This function iterates through a vector of `character_data` and determines
  /// the index of the element with the minimum distance based on a specific
  /// criterion. If the vector is empty or no valid index can be determined, it returns an empty optional.
  /// \param char_data A vector of `character_data` objects to evaluate.
  /// \return The index of the element with the minimum distance, or an empty optional if no valid index is found.
  ///
  auto min_distance_index(const std::vector<character_data>& char_data) const -> std::optional<std::size_t>;

  ///
  /// Get the bounding boxes of each recognized line and the index of the line containing the cursor.
  /// \param relative_cursor_position The screen coordinates of the cursor position in the image.
  /// \return Optional line position data, std::nullopt if no line contains the cursor position.
  ///
  auto find_line_position_data(const screen_coordinates_type& relative_cursor_position) const
    -> std::optional<line_position_data>;

private: // Variables
  const std::unique_ptr<core::core_tesseract> core_tesseract_;
};

} // namespace bibstd::core
