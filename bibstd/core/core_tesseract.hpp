#pragma once

#include "core/core_tesseract_common.hpp"
#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "math/rect.hpp"
#include "system/filesystem.hpp"

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
  static constexpr std::string_view log_channel = "core_tesseract";
  static constexpr std::string_view tessdata_folder_name = "tessdata";
  inline static const std::filesystem::path tessdata_folder_path{
    system::filesystem::executable_folder() / tessdata_folder_name
  };

public: // Typedefs
  using pixel_plane_type = core_tesseract_common::pixel_plane_type;
  using bounding_box_type = core_tesseract_common::bounding_box_type;

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
  auto recognize(std::optional<bounding_box_type> bounding_box) const -> bool;

  ///
  /// Run tesseract OCR on image.
  /// \param resolution Text resolution
  /// \param do_with_text Callback that is called for each parse found
  ///
  auto for_each(
    text_resolution resolution, const std::function<void(std::string_view, const bounding_box_type&)>& do_with_text
  ) const -> void;

  ///
  /// Run tesseract OCR on image with breakout condition.
  /// \param resolution Text resolution
  /// \param do_with_text Callback that is called for each parse found. If function returns true, the parsing is stopped.
  ///
  auto for_each_until(
    text_resolution resolution, const std::function<bool(std::string_view, const bounding_box_type&)>& do_with_text
  ) const -> void;

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
