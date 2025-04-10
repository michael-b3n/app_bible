#pragma once

#include "bible/reference_range.hpp"
#include "util/exception.hpp"

#include <optional>
#include <string_view>

namespace bibstd::bible
{

class reference_formatter final
{
public: // Typedefs
  using translations = std::vector<translation>;

public: // Operations
  ///
  /// Formats a reference range to pretty format.
  /// \param range Reference range that shall be formatted
  /// \return formatted reference range as string
  ///
  auto pretty(const reference_range& range) -> std::string;

private: // Constants
  static constexpr std::string_view log_channel = "reference_formatter";
};

} // namespace bibstd::bible
