#pragma once

#include "bible/reference_range.hpp"
#include "math/value_range.hpp"
#include "txt/chars.hpp"

#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace bibstd::core
{

///
/// Core bible verse. This class searches strings and identifies bible verses.
///
class core_bible_reference final
{
public: // Typedefs
  using index_range_type = math::value_range<std::size_t>;
  struct parse_result final
  {
    std::vector<bible::reference_range> ranges{};
    index_range_type index_range_origin{0, 0};
  };

public: // Constants
  ///
  /// List of all characters which can be used to separate passage numbers.
  ///
  static constexpr auto transition_chars = std::array{',', ';', '.', '-', ':'};

  ///
  /// List of all postfixes which can appear in a bible reference after a number.
  /// \note Adjust the list below if more postfixes are added.
  ///
  static constexpr auto number_postfixes = std::array{
    std::string_view("ff."),
    std::string_view("f."),
    std::string_view("a"),
    std::string_view("b"),
    std::string_view("c"),
    std::string_view("d")
  };

  ///
  /// List of all letters appearing in `number_postfixes`.
  ///
  static constexpr auto number_postfix_chars = std::array{'f', 'a', 'b', 'c', 'd'};

public: // Structors
  core_bible_reference() = default;

public: // Modifiers
  ///
  /// Parse bible reference from a string view. This function will only parse a reference of a single book.
  /// \param text String view containing bible references
  /// \param index Index where the bible reference shall be
  /// \return parse result with bible reference ranges and origin text
  ///
  auto parse(std::string_view text, std::size_t index) const -> parse_result;

private: // Typedefs
  using passage_template_value_type = std::variant<std::uint32_t, char>;
  using passage_template_type = std::vector<passage_template_value_type>;

  enum class passage_level
  {
    chapter,
    verse,
  };

  struct find_book_result final
  {
    bible::book_id book_id;
    math::value_range<std::size_t> index_range_book;
    math::value_range<std::size_t> index_range_numbers;
    std::string_view book_name_variant;
  };

  struct passage_section final
  {
    std::vector<std::uint32_t> numbers;
    std::string generic_template;
  };

private: // Implementation
  ///
  /// Find the book name in the text at the given index. The book name is identified by the book name variants.
  /// \param text Text to search for the book name
  /// \param index Index where the book name shall be
  /// \return Book name and index range of the book name and numbers or std::nullopt if no book name is found
  ///
  auto find_book(std::string_view text, std::size_t index) const -> std::optional<find_book_result>;

  ///
  /// Find the numbers after the book name that are possibly part of the reference.
  /// Note that the numbers are not guaranteed to be part of the reference, it might be part of another book or a list.
  /// \param text_after_name Text after the book name
  /// \return Index of the last number in the text after the book name or std::nullopt if no number is found
  ///
  auto find_numbers_after_book_name(std::string_view text_after_name) const -> std::optional<std::size_t>;

  ///
  /// It is possible that the last number found for the number index range in find_book_result belongs to another book.
  /// If this is the case we remove the last number from the numbers index range. This function checks if the last number
  /// belongs to the book name variant and returns the new index range.
  /// \param text_after_name Text to check if the last number belongs to the book name variant
  /// \param numbers_end Index of the last number in the index range
  /// \return Validated index range for the numbers
  ///
  auto validate_index_range_numbers_end(std::string_view text_after_name, std::size_t numbers_end) const -> std::size_t;

  ///
  /// Create a passage template from a string view. The passage template is a vector of numbers and transition characters.
  /// \param passage_text String view from which the passage template is created
  /// \return Passage template with numbers and transition characters
  ///
  auto create_passage_template(std::string_view passage_text) const -> passage_template_type;

  ///
  /// Normalize the passage text by removing all characters that are not part of the passage template.
  /// \param text String view to normalize
  /// \return Normalized passage text
  ///
  auto normalize_passage_text(std::string_view text) const -> std::string;

  ///
  /// Helper to identify the numbers in the text at the given index.
  /// If a number is found, the index is updated to the next character after the number.
  /// \param text Text to search for the numbers
  /// \param pos Index where the numbers shall be
  /// \return Number at the given index or std::nullopt if no number is found
  ///
  auto identify_number(std::string_view text, std::size_t& pos) const -> std::optional<std::uint32_t>;

  ///
  /// Helper to identify the transition character in the text at the given index.
  /// If a transition character is found, the index is updated to the next character after the transition character.
  /// \param text Text to search for the transition character
  /// \param pos Index where the transition character shall be
  /// \return Transition character at the given index or std::nullopt if no transition character is found
  ///
  auto identify_transition(std::string_view text, std::size_t& pos) const -> std::optional<char>;

  ///
  /// Match a passage template section and return the corresponding reference ranges.
  /// \param book Book ID of the bible reference
  /// \param passage_template Passage template with numbers and transition characters
  /// \return Vector of reference ranges matching the passage template section
  ///
  auto match_passage_template(bible::book_id book, passage_template_type&& passage_template) const
    -> std::vector<bible::reference_range>;

  ///
  /// Create a list of chars from the passage template.
  /// \param passage_template Passage template with numbers and transition characters
  /// \return Vector of chars corresponding to the passage template
  ///
  auto passage_template_transition_chars(const passage_template_type& passage_template) const -> std::vector<char>;

  ///
  /// Create a list of numbers from the passage template.
  /// \param passage_template Passage template with numbers and transition characters
  /// \return Vector of numbers corresponding to the passage template
  ///
  auto passage_template_numbers(const passage_template_type& passage_template) const -> std::vector<std::uint32_t>;

  ///
  /// Create a list of passage sections from the passage template.
  /// \param passage_template Passage template with numbers and transition characters
  /// \param down_transition_char Transition character to use as separations between the passage sections
  /// \return Vector of passage sections
  ///
  auto create_passage_sections(const passage_template_type& passage_template, std::optional<char> down_transition_char) const
    -> std::vector<passage_section>;
};

} // namespace bibstd::core
