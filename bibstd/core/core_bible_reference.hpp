#pragma once

#include "bible/reference_parser.hpp"
#include "bible/reference_range.hpp"

#include <optional>
#include <string_view>
#include <vector>

namespace bibstd::core
{

///
/// Core bible verse. This class searches strings and identifies bible verses.
///
class core_bible_reference final
{
public: // Constants
  static constexpr std::string_view log_channel = "core_bible_reference";

public: // Typedefs

public: // Structors
  core_bible_reference() = default;

public: // Modifiers
  ///
  /// Set image to recognize.
  /// \param bitmap Bitmap that shall be recognize
  ///
  auto find(std::string_view text) -> std::vector<bible::reference_range>;

private: // Variables
  bible::reference_parser parser_;
};

} // namespace bibstd::core
