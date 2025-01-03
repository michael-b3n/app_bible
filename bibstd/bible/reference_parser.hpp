#pragma once

#include "bible/reference_range.hpp"

#include <expected>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace bibstd::bible
{

///
/// Parses bible references from a string view.
///
class reference_parser final
{
public: // Constructor
  reference_parser() = default;

public: // Operations
  ///
  /// Parse bible reference from a string view.
  /// \param text String view containing bible references
  /// \return Parsed bible reference ranges
  ///
  auto parse(std::string_view text) -> std::vector<reference_range>;

private: // Typedefs
  enum class passage_level
  {
    chapter,
    verse,
  };

private: // Constants
  static constexpr std::string_view log_channel = "reference_parser";

private: // Implementation
  auto cache_normalized_without_whitespaces(std::string_view text) -> void;
  auto identify_number(std::string_view text, std::size_t& pos) const -> std::optional<std::uint32_t>;
  auto skip_number_postfix(std::string_view text, std::size_t& pos) const -> void;
  auto identify_transition(std::string_view text, std::size_t& pos) const -> std::optional<char>;
  auto match_passage_template(book_id book, const std::vector<std::uint32_t>& numbers) -> std::vector<reference_range>;
  auto normalize_passage_template() -> void;

private: // Variables
  std::string text_;
  std::string passage_template_;
  std::string passage_template_normalized_;
  std::string down_transition_chars_;
  std::map<char, std::vector<reference_range>> reference_ranges_;
  std::vector<std::pair<char, std::uint32_t>> reference_ranges_verse_count_;
};

} // namespace bibstd::bible
