#pragma once

#include "bibstd/bible/parser.hpp"
#include "bibstd/bible/reference.hpp"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace bibstd::bible
{
// Forward declarations
class parser;
} // namespace bibstd::bible
namespace bibstd::io
{
// Forward declarations
class zip_file_reader;
} // namespace bibstd::io
namespace bibstd::core
{

///
/// Core scripture store. This class contains loaded scripture data.
///
class core_scripture_store final
{
public: // Typedefs
  using scripture_info = bible::parser::scripture_info;
  using html_passage = bible::parser::html_passage;

  ///
  /// Supported file types for scripture data.
  ///
  enum class supported_file_type
  {
    zip,
  };

  ///
  /// Supported scripture formats.
  ///
  enum class supported_format_type
  {
    usx,
  };

public: // Structors
  core_scripture_store();
  ~core_scripture_store() noexcept;

public: // Accessors
  ///
  /// Get the available scriptures in the store.
  /// \return A vector of scripture IDs for all available scriptures
  ///
  auto scripture_names() const -> std::vector<std::string>;

  ///
  /// Get information about a specific scripture.
  /// \param id The ID of the scripture to retrieve
  /// \return The information of the scripture, or an empty optional if not found
  ///
  auto info(const std::string& name) const -> std::optional<scripture_info>;

  ///
  /// Get the HTML content of a specific passage.
  /// \param name The name of the scripture to retrieve
  /// \param ref The reference defining the passage to get
  /// \return The HTML content of the passage, or an error code if retrieval fails
  ///
  auto passage_html(const std::string& name, const bible::reference& ref) const -> std::optional<html_passage>;

private: // Implementation
  auto load_usx(const io::zip_file_reader& zip_reader) -> bool;

private: // Variables
  std::map<std::string, std::unique_ptr<bible::parser>> scripture_data_;
};

} // namespace bibstd::core
