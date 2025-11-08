#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/core/core_bible_ref_ocr_common.hpp"
#include "bibstd/core/core_tesseract_common.hpp"
#include "bibstd/math/value_range.hpp"
#include "bibstd/txt/indexed_strings.hpp"
#include "bibstd/util/screen_types.hpp"

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
class core_bible_ref_ocr final
{
public: // Typedefs
  using screen_rect_type = util::screen_types::screen_rect_type;
  using screen_coordinates_type = util::screen_types::screen_coordinates_type;
  using pixel_plane_type = util::screen_types::pixel_plane_type;
  using tesseract_choice = core_tesseract_common::tesseract_choice;
  using tesseract_choices = core_tesseract_common::tesseract_choices;
  using character_data = core_bible_ref_ocr_common::character_data;
  using reference_position_data = core_bible_ref_ocr_common::reference_position_data;

  struct recognize_bounding_box_result final
  {
    screen_rect_type reduced;
    screen_rect_type largest;
  };

  struct capture_screen_result final
  {
    screen_coordinates_type relative_cursor_pos;
    pixel_plane_type image;
  };

public: // Structors
  core_bible_ref_ocr(const std::filesystem::path& tessdata_path, core_tesseract_common::language language);
  ~core_bible_ref_ocr() noexcept;

public: // Operations
  ///
  /// Capture an area of the screen. This function does not modify core tesseract.
  /// \param cursor_position Cursor position on the screen
  /// \return optional relative cursor position and captured pixel plane, std::nullopt if capturing image failed
  ///
  [[nodiscard]] auto capture_screen(const screen_coordinates_type& cursor_position) const
    -> std::optional<capture_screen_result>;

  ///
  /// Find the bounding box of the lines containing the given cursor position with the reference.
  /// If no paragraph is found at the specified position, returns std::nullopt
  /// \param image_data The relative cursor position and the pixel plane containing the image data
  /// \return An optional screen rectangle representing the bounding box of the paragraph.
  /// If no paragraph is found, returns std::nullopt.
  ///
  [[nodiscard]] auto recognize_bounding_box(capture_screen_result&& image_data) const
    -> std::optional<recognize_bounding_box_result>;

  ///
  /// Recognize capture area using OCR. The functions generates the area depending on the structural
  /// recognized bounding box.
  /// \param recognized_bounding_box Structural recognized bounding box
  /// \param recognize_largest_bounding_box If true, the largest bounding box is used for the recognition
  /// \return true if the OCR recognition was successful, otherwise false
  ///
  [[nodiscard]] auto recognize_capture_area(
    const recognize_bounding_box_result& recognized_bounding_box, bool recognize_largest_bounding_box
  ) const -> bool;

  ///
  /// Finds the main reference position data based on the given cursor position.
  /// This function takes the screen coordinates of a cursor position and attempts to
  /// determine the corresponding reference position data. If a valid reference position
  /// is found, it returns the data wrapped in a std::optional. Otherwise, it returns
  /// an empty std::optional.
  /// \param relative_cursor_pos The screen coordinates of the cursor position in the image.
  /// \return std::optional<reference_position_data> The reference position data if found,
  /// otherwise an empty std::optional.
  ///
  auto find_main_reference_position_data(const screen_coordinates_type& relative_cursor_pos) const
    -> std::optional<reference_position_data>;

  ///
  /// Finds reference position data based on the given cursor position.
  /// This function analyzes the provided cursor position and determines the
  /// corresponding reference position data from a set of OCR choices. The result
  /// is a list of possible `reference_position_data` objects.
  /// \param relative_cursor_pos The screen coordinates of the cursor position in the image.
  /// \return A vector containing the reference position data associated with the given cursor position.
  ///
  auto find_reference_position_data_from_choices(const screen_coordinates_type& relative_cursor_pos) const
    -> std::vector<reference_position_data>;

private: // Typedefs
  struct line_position_data final
  {
    std::vector<screen_rect_type> line_bounding_boxes;
    std::size_t cursor_line_index;
  };

private: // Constants
  // Area validation constants
  static constexpr auto area_validation_horizontal_margin_multiplier = 2.0;
  static constexpr auto area_validation_vertical_margin_multiplier = 0.2;

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
  /// \return vector of confidence sums and indexed strings with the text template symbols inserted at possible positions
  /// The index count is guaranteed to be the same as the number of choices in the choices_list.
  ///
  [[nodiscard]] auto match_choices_to_string(
    const std::vector<tesseract_choices>& choices_list,
    std::string_view text_template,
    const std::function<bool(const tesseract_choices&)>& choices_filter
  ) const -> std::vector<std::pair<double, txt::indexed_strings>>;

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
  /// \param relative_cursor_pos The screen coordinates of the cursor position in the image.
  /// \return Optional line position data, std::nullopt if no line contains the cursor position.
  ///
  auto find_line_position_data(const screen_coordinates_type& relative_cursor_pos) const -> std::optional<line_position_data>;

private: // Variables
  const std::unique_ptr<core::core_tesseract> core_tesseract_;
};

} // namespace bibstd::core
