#pragma once

#include "bible/reference_range.hpp"
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
public: // Constants
  static constexpr std::string_view log_channel = "core_bible_reference_ocr";

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
  auto find_chars_begin_match(const tesseract_choices& choices, std::string_view chars) const
    -> std::optional<tesseract_choice>;
};

} // namespace bibstd::core
