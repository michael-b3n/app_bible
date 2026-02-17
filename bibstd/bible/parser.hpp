#pragma once

#include "bibstd/bible/passage.hpp"
#include "bibstd/bible/passage_info.hpp"

#include <expected>
#include <optional>

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
    not_found,     ///< Passage not found
    access_denied, ///< Access to passage denied
    invalid_range, ///< Invalid passage range provided
    unknown        ///< Unknown error occurred
  };

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
  auto passage_html(const bible::passage_info& info) const -> std::expected<bible::passage_html, error_code>;

private: // Implementation
  virtual auto do_valid() const -> bool = 0;
  virtual auto do_info() const -> scripture_info = 0;
  virtual auto do_passage_html(const bible::passage_info& info) const -> std::expected<bible::passage_html, error_code> = 0;
};

} // namespace bibstd::bible
