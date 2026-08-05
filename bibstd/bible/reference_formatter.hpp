#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/util/language.hpp"

#include <string>

namespace bibstd::bible
{

struct reference_formatter final
{
  // Operators
  ///
  /// Formats a reference range to pretty format.
  /// \return formatted reference range as string
  ///
  static auto operator()(util::language language, const reference_range& range) -> std::string;
};

} // namespace bibstd::bible
