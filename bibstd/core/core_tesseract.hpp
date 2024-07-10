#pragma once

#include "util/bitmap.hpp"

#include <string_view>

namespace tesseract
{
class TessBaseAPI;
} // namespace tesseract

namespace bibstd::core
{

///
/// Core tesseract.
///
class core_tesseract final
{
public: // Constants
  static constexpr std::string_view log_channel = "core_tesseract";
  static constexpr std::string_view language_de = "deu";

public: // Structors
  core_tesseract(std::string_view language);
  ~core_tesseract() noexcept;

public: // Accessor

public: // Modifiers
  auto load(util::bitmap&& bitmap) -> void;
  auto text() -> std::string;

private: // Implementation

private: // Variables
  std::shared_ptr<tesseract::TessBaseAPI> tesseract_;
  util::bitmap bitmap_;
};

} // namespace bibstd::core
