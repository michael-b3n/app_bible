#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace bibstd::core
{

///
/// Core tesseract common. This helper struct contains helper types for core tesseract.
///
struct core_tesseract_common final
{
  // Typedefs
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
