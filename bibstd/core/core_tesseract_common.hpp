#pragma once

#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "math/rect.hpp"
#include "util/const_bimap.hpp"
#include "util/screen_types.hpp"

#include <filesystem>
#include <optional>
#include <string>

namespace bibstd::core
{

///
/// Core tesseract common. This helper struct contains helper types for core tesseract.
///
struct core_tesseract_common final
{
  // Typedefs
  enum class language
  {
    de,
  };

  struct tesseract_choice final
  {
    std::string symbol{""};
    double confidence{0.0};
  };
  using tesseract_choices = std::vector<tesseract_choice>;

  // Static functions
  ///
  /// Find tessdata folder by searching from executable folder upwards.
  /// \return optional path to tessdata folder, std::nullopt if not found
  ///
  [[nodiscard]] static auto tessdata_folder_finder() -> std::optional<std::filesystem::path>;
};

} // namespace bibstd::core
