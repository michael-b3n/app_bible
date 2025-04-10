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
public: // Constants
  static constexpr std::string_view log_channel = "core_bible_reference";

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
  /// Parse bible reference from a string view.
  /// \param text String view containing bible references
  /// \param index Index where the bible reference shall be
  /// \return parsed bible reference ranges
  ///
  auto parse(std::string_view text, std::size_t index) -> std::vector<bible::reference_range>;

private: // Typedefs
  using index_range_type = math::value_range<std::size_t>;
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
    math::value_range<std::size_t> index_range_numbers;
    std::string_view book_name_variant;
  };

  struct passage_section final
  {
    std::vector<std::uint32_t> numbers;
    std::string generic_template;
  };

private: // Implementation
  auto find_book(std::string_view text, std::size_t index) -> std::optional<find_book_result>;
  auto create_passage_template(std::string_view passage_text) -> passage_template_type;
  auto normalize_passage_text(std::string_view text) -> std::string;
  auto identify_number(std::string_view text, std::size_t& pos) const -> std::optional<std::uint32_t>;
  auto identify_transition(std::string_view text, std::size_t& pos) const -> std::optional<char>;
  auto match_passage_template(bible::book_id book, passage_template_type&& passage_template)
    -> std::vector<bible::reference_range>;
  auto create_passage_sections(const passage_template_type& passage_template, char down_transition_char)
    -> std::vector<passage_section>;
};

} // namespace bibstd::core
