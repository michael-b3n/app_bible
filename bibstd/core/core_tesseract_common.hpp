#pragma once

#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "math/rect.hpp"
#include "util/const_bimap.hpp"
#include "util/screen_types.hpp"

#include <string_view>

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
};

} // namespace bibstd::core
