#pragma once

#include "bibstd/bible/reference.hpp"

#include <optional>
#include <string>

namespace bibstd::bible
{

///
/// Virtual base class for the bible passage backend.
/// This class provides an interface for accessing bible passages from various sources.
///
class parser
{
public: // Typedefs
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

  ///
  /// Error codes for passage access operations.
  ///
  enum class error_code
  {
    not_found, ///< Passage not found
    unknown    ///< Unknown error occurred
  };

public: // Constants
  static constexpr std::string_view html_bold = "b";
  static constexpr std::string_view html_italic = "i";
  static constexpr std::string_view html_paragraph = "p";
  static constexpr std::string_view html_superscript = "sup";
  static constexpr std::string_view html_h3 = "h3";

  static constexpr std::string_view format_chapter_number = html_h3;
  static constexpr std::string_view format_verse_number = html_superscript;
  static constexpr std::string_view format_paragraph = html_paragraph;
  static constexpr std::string_view format_name_of_god = html_italic;
  static constexpr std::string_view format_emphasized = html_italic;
  static constexpr std::string_view format_translator_addition = html_italic;

public: // Constructor
  parser();
  virtual ~parser() noexcept;

public: // Accessors
  ///
  /// Check if the passage reader is valid.
  /// \return true if valid, false otherwise
  ///
  auto valid() const -> bool;

  ///
  /// Access information about the scripture.
  /// \return Information about the scripture
  ///
  auto info() const -> scripture_info;

  ///
  /// Get a bible passage.
  /// \param reference The reference defining the passage to get.
  /// \return Expected passage, or an error code
  ///
  auto passage_html(const reference& ref) const -> std::expected<html_passage, error_code>;

private: // Implementation
  virtual auto do_valid() const -> bool = 0;
  virtual auto do_info() const -> scripture_info = 0;
  virtual auto do_passage_html(const reference& ref) const -> std::expected<html_passage, error_code> = 0;
};

} // namespace bibstd::bible
