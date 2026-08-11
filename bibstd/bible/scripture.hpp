#pragma once

#include "bibstd/bible/reference.hpp"
#include "bibstd/bible/versification.hpp"

#include <optional>
#include <string>
#include <vector>

namespace bibstd::bible
{

///
/// Virtual base class for the bible passage backend.
/// This class provides an interface for accessing bible passages from various sources.
/// \note The implementation of this interface shall be thread-safe.
///
class scripture
{
  // Typedefs
  ///
  /// Struct containing the HTML content of a bible verse,
  /// with paragraph markers using data-id attribute ("begin", "continue", or "undefined").
  ///
  template<typename Tag>
  struct passage final
  {
    // Operators
    auto operator==(const passage&) const -> bool = default;

    // Variables
    reference ref; // from the parsed scripture
    std::string content;
    std::vector<std::string> cross_references;
  };

  ///
  /// Information about a scripture.
  ///
  struct info final
  {
    // Operators
    auto operator==(const info&) const -> bool = default;

    // Variables
    std::string name;
    std::string abbreviation;
    std::string language;
    std::optional<std::string> copyright;
  };

  ///
  /// Names of a single book in the language of the scripture.
  /// Not every scripture provides all of the forms, unavailable ones are empty.
  ///
  struct book_name final
  {
    // Operators
    auto operator==(const book_name&) const -> bool = default;

    // Variables
    std::string abbreviation; // abbreviated form, e.g. "1. Mose"
    std::string short_name;   // form intended for display, e.g. "1. Mose"
    std::string long_name;    // full title, e.g. "Das 1. Buch Mose (Genesis)"
  };

public: // Typedefs
  using reference_type = reference;
  using versification_type = versification;
  using passage_html_type = passage<struct html_tag>;
  using info_type = info;
  using book_name_type = book_name;

public: // Constants
  static constexpr std::string_view html_format_name_of_god = "i";
  static constexpr std::string_view html_format_translator_addition = "i";

  static constexpr std::string_view html_custom_attr_name_id = "data-id";
  static constexpr std::string_view html_custom_attr_value_p_begin = "begin";
  static constexpr std::string_view html_custom_attr_value_p_continue = "continue";
  static constexpr std::string_view html_custom_attr_value_p_undefined = "undefined";

public: // Constructor
  scripture();
  virtual ~scripture() noexcept;

public: // Accessors
  ///
  /// Access information about the scripture.
  /// \return Information about the scripture
  ///
  auto information() const -> info_type;

  ///
  /// Access the names of a book in the language of the scripture.
  /// \param book Book to get the names of
  /// \return Names of the book, or std::nullopt if the scripture does not provide them
  ///
  auto book_information(book_id book) const -> std::optional<book_name_type>;

  ///
  /// Get a bible passage of the specified reference.
  /// \return Expected passage, or std::nullopt if not found
  ///
  auto passage_html(const reference_type& ref) const -> std::optional<passage_html_type>;

  ///
  /// Get the versification associated with this scripture.
  /// \return versification
  ///
  auto versification() const -> const versification_type&;

private: // Implementation
  virtual auto do_information() const -> info_type = 0;
  virtual auto do_book_information(book_id book) const -> std::optional<book_name_type> = 0;
  virtual auto do_passage_html(const reference_type& ref) const -> std::optional<passage_html_type> = 0;
  virtual auto do_versification() const -> const versification_type& = 0;
};

} // namespace bibstd::bible
