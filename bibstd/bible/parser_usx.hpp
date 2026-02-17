#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/parser.hpp"

#include <map>
#include <memory>

namespace pugi
{
// Forward declarations
class xml_document;
} // namespace pugi
namespace bibstd::io
{
// Forward declarations
class zip_file_reader;
} // namespace bibstd::io
namespace bibstd::bible
{

///
/// Scripture parser for USX formatted files.
///
class parser_usx final : public parser
{
public: // Constants
  static constexpr auto unknown_name = "Unknown Scripture";
  static constexpr auto unknown_abbreviation = "Unknown Abbreviation";
  static constexpr auto unknown_language = "Unknown Language";

public: // Typedefs
  using book_data_type = std::map<book_id, std::unique_ptr<pugi::xml_document>>;

public: // Constructor
  parser_usx(const io::zip_file_reader& zip_reader);
  ~parser_usx() noexcept override;

private: // Overrides
  ///
  /// \see parser::valid
  ///
  auto do_valid() const -> bool override;

  ///
  /// \see parser::info
  ///
  auto do_info() const -> scripture_info override;

  ///
  /// \see parser::passage_html
  ///
  auto do_passage_html(const bible::passage_info& info) const -> std::expected<bible::passage_html, error_code> override;

private: // Implementation

private: // Variables
  const std::optional<scripture_info> info_data_;
  const book_data_type book_data_;
};

} // namespace bibstd::bible
