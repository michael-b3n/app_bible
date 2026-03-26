#pragma once

#include <optional>
#include <string>

namespace bibstd::bible
{

///
/// Common structures and definitions for bible parsers.
///
struct parser_common final
{
  // Typedefs
  ///
  /// Information about a scripture.
  ///
  struct scripture_info final
  {
    // Operators
    auto operator==(const scripture_info&) const -> bool = default;

    // Variables
    std::string name;
    std::string abbreviation;
    std::string language;
    std::optional<std::string> copyright;
  };

  ///
  /// Struct containing the HTML content of a bible passage,
  /// along with the begin index of the first verse in the passage.
  ///
  struct html_passage final
  {
    // Operators
    auto operator==(const html_passage&) const -> bool = default;

    // Variables
    std::size_t begin_index;
    std::string content;
  };
};

} // namespace bibstd::bible
