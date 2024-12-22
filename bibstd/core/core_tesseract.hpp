#pragma once

#include "math/rect.hpp"
#include "system/filesystem.hpp"
#include "util/bitmap.hpp"

#include <string_view>

// Forward declarations
namespace tesseract
{
class TessBaseAPI;
} // namespace tesseract

namespace bibstd::core
{

///
/// Core tesseract. This class uses OCR on loaded bitmaps.
///
class core_tesseract final
{
public: // Constants
  static constexpr std::string_view log_channel = "core_tesseract";
  static constexpr std::string_view language_de = "deu";
  static constexpr std::string_view tessdata_folder_name = "tessdata";
  inline static const std::filesystem::path tessdata_folder_path{system::filesystem::executable_folder() / tessdata_folder_name};

public: // Typedefs
  using bounding_box_type = math::rect<std::uint32_t>;

public: // Structors
  core_tesseract(std::string_view language);
  ~core_tesseract() noexcept;

public: // Modifiers
  ///
  /// Set image to recognize.
  /// \param bitmap Bitmap that shall be recognize
  ///
  auto set_image(util::bitmap&& bitmap) -> void;

  ///
  /// Run tesseract OCR on image.
  /// \param callback Callback that is called for each word found
  ///
  auto for_each_word(std::function<void(std::string_view, const bounding_box_type&)> callback) -> void;

private: // Variables
  std::unique_ptr<tesseract::TessBaseAPI> tesseract_;
  util::bitmap bitmap_;
};

} // namespace bibstd::core
