#pragma once

#include "bibstd/bible/parser_common.hpp"

#include <memory>
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
  using scripture_info = bible::parser_common::scripture_info;
  using passage_info = bible::parser_common::passage_info;
  using html_passage = bible::parser_common::html_passage;

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
  /// \return A vector of scripture information for all available scriptures
  ///
  auto available_scriptures() const -> std::vector<scripture_info>;

  ///
  /// Get the HTML content of a specific passage.
  /// \param scripture Information about the scripture to retrieve
  /// \param passage Information about the passage to retrieve
  /// \return The HTML content of the passage, or an error code if retrieval fails
  ///
  auto passage_html(const scripture_info& scripture, const passage_info& passage) const -> std::optional<html_passage>;

private: // Implementation
  auto load_usx(const io::zip_file_reader& zip_reader) -> bool;

private: // Variables
  std::vector<std::unique_ptr<bible::parser>> scripture_data_;
};

} // namespace bibstd::core
