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
private: // Typedefs
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

public: // Typedefs
  using reference_type = reference;
  using versification_type = versification;
  using passage_html_type = passage<struct html_tag>;
  using info_type = info;

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
  /// Get a bible passage.
  /// \param reference The reference defining the passage to get.
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
  virtual auto do_passage_html(const reference_type& ref) const -> std::optional<passage_html_type> = 0;
  virtual auto do_versification() const -> const versification_type& = 0;
};

} // namespace bibstd::bible
