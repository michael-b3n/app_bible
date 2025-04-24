#pragma once

#include "core/core_tesseract_common.hpp"
#include "system/filesystem.hpp"
#include "util/screen_types.hpp"

#include <optional>
#include <string_view>

// Forward declarations
namespace tesseract
{
class TessBaseAPI;
} // namespace tesseract
namespace bibstd::data
{
class pix;
} // namespace bibstd::data
namespace bibstd::core
{

///
/// Core tesseract. This class uses OCR on loaded bitmaps.
///
class core_tesseract final
{
public: // Constants
  static constexpr std::string_view tessdata_folder_name = "tessdata";
  inline static const std::filesystem::path tessdata_folder_path{
    system::filesystem::executable_folder() / tessdata_folder_name
  };

public: // Typedefs
  using screen_rect_type = util::screen_types::screen_rect_type;
  using screen_coordinates_type = util::screen_types::screen_coordinates_type;
  using pixel_plane_type = util::screen_types::pixel_plane_type;
  using text_callback_type = std::function<void(std::string_view, const screen_rect_type&)>;
  using text_while_callback_type = std::function<bool(std::string_view, const screen_rect_type&)>;
  using tesseract_choice = core_tesseract_common::tesseract_choice;
  using tesseract_choices = core_tesseract_common::tesseract_choices;

  // clang-format off
  using choices_callback_type = std::function<void(const tesseract_choices&, const screen_rect_type&)>;
  using choices_while_callback_type = std::function<bool(const tesseract_choices&, const screen_rect_type&)>;
  // clang-format on

  enum class text_resolution
  {
    character,
    word,
    line,
    paragraph,
  };

public: // Structors
  core_tesseract(core_tesseract_common::language language);
  ~core_tesseract() noexcept;

public: // Modifiers
  ///
  /// Set image that shall be recognized with tesseract.
  /// \param setter Function that sets the image that shall be read with tesseract
  ///
  auto set_image(const std::function<void(pixel_plane_type&)>& setter) -> void;

  ///
  /// Recognize image or sub-rectangle of image with tesseract.
  /// \param bounding_box Optional rectangle within image to recognize, if not set the whole image is recognized.
  /// \return true if recognition was successful, false otherwise
  ///
  auto recognize(std::optional<screen_rect_type> bounding_box) const -> bool;

  ///
  /// Run tesseract OCR on image.
  /// \param resolution Text resolution
  /// \param do_with_text Callback that is called for each parse found
  ///
  auto for_each(text_resolution resolution, const text_callback_type& do_with_text) const -> void;

  ///
  /// Run tesseract OCR on image with breakout condition.
  /// \param resolution Text resolution
  /// \param do_with_text Callback that is called for each parse found. If function returns true, the parsing is stopped.
  ///
  auto for_each_while(text_resolution resolution, const text_while_callback_type& do_with_text) const -> void;

  ///
  /// Run tesseract OCR on image.
  /// \param do_with_choices Callback that is called for each symbol and its choices found. The choices are guaranteed to be non
  /// empty and sorted by confidence.
  ///
  auto for_each_choices(const choices_callback_type& do_with_choices) const -> void;

  ///
  /// Run tesseract OCR on image with breakout condition.
  /// \param do_with_choices Callback that is called for each symbol and its choices found. The choices are guaranteed to be non
  /// empty and sorted by confidence. If function returns true, the parsing is stopped.
  ///
  auto for_each_choices_while(const choices_while_callback_type& do_with_choices) const -> void;

private: // Constants
  static constexpr auto language_map = util::const_bimap{
    std::pair{core_tesseract_common::language::de, std::string_view("deu")},
    // ...
  };

private: // Variables
  const std::unique_ptr<tesseract::TessBaseAPI> tesseract_;
  const std::unique_ptr<data::pix> pix_;
};

} // namespace bibstd::core
