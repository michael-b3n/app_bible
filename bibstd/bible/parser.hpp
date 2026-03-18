#pragma once

#include "bibstd/bible/parser_common.hpp"

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
  /// Error codes for passage access operations.
  ///
  enum class error_code
  {
    not_found, ///< Passage not found
    unknown    ///< Unknown error occurred
  };

  using scripture_info = parser_common::scripture_info;
  using passage_info = parser_common::passage_info;
  using html_passage = parser_common::html_passage;

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
  /// \param info The passage info defining the passage to get.
  /// \return Expected passage, or an error code
  ///
  auto passage_html(const passage_info& info) const -> std::expected<html_passage, error_code>;

private: // Implementation
  virtual auto do_valid() const -> bool = 0;
  virtual auto do_info() const -> scripture_info = 0;
  virtual auto do_passage_html(const passage_info& info) const -> std::expected<html_passage, error_code> = 0;
};

} // namespace bibstd::bible
