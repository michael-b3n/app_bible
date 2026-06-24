#pragma once

#include "bibstd/txt/ocr_engine.hpp"
#include "bibstd/util/language.hpp"

namespace bibstd::system
{

///
/// System OCR interface. Provides platform-specific OCR engine interface and capability queries.
///
struct ocr final
{
  // Static
  ///
  /// Create a system OCR engine instance. The creator will return std::monostate
  /// if the system backend fails to create or does not support an OCR engine.
  /// \param language Language for recognition
  /// \return unique pointer to the created OCR engine, or nullptr if not available
  ///
  [[nodiscard]] static auto create(util::language language) -> txt::ocr_engine_uptr_variant_type;
};

} // namespace bibstd::system
