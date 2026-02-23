#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/parser.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/util/enum.hpp"

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

  static constexpr auto books = util::make_const_bimap<book_id, std::string_view>({
    {        book_id::genesis, "GEN"},
    {         book_id::exodus, "EXO"},
    {      book_id::leviticus, "LEV"},
    {        book_id::numbers, "NUM"},
    {    book_id::deuteronomy, "DEU"},
    {         book_id::joshua, "JOS"},
    {         book_id::judges, "JDG"},
    {           book_id::ruth, "RUT"},
    {        book_id::samuel1, "1SA"},
    {        book_id::samuel2, "2SA"},
    {         book_id::kings1, "1KI"},
    {         book_id::kings2, "2KI"},
    {    book_id::chronicles1, "1CH"},
    {    book_id::chronicles2, "2CH"},
    {           book_id::ezra, "EZR"},
    {       book_id::nehemiah, "NEH"},
    {         book_id::esther, "EST"},
    {            book_id::job, "JOB"},
    {         book_id::psalms, "PSA"},
    {       book_id::proverbs, "PRO"},
    {   book_id::ecclesiastes, "ECC"},
    {book_id::song_of_solomon, "SNG"},
    {         book_id::isaiah, "ISA"},
    {       book_id::jeremiah, "JER"},
    {   book_id::lamentations, "LAM"},
    {        book_id::ezekiel, "EZK"},
    {         book_id::daniel, "DAN"},
    {          book_id::hosea, "HOS"},
    {           book_id::joel, "JOL"},
    {           book_id::amos, "AMO"},
    {        book_id::obadiah, "OBA"},
    {          book_id::jonah, "JON"},
    {          book_id::micah, "MIC"},
    {          book_id::nahum, "NAM"},
    {       book_id::habakkuk, "HAB"},
    {      book_id::zephaniah, "ZEP"},
    {         book_id::haggai, "HAG"},
    {      book_id::zechariah, "ZEC"},
    {        book_id::malachi, "MAL"},
    {        book_id::matthew, "MAT"},
    {           book_id::mark, "MRK"},
    {           book_id::luke, "LUK"},
    {           book_id::john, "JHN"},
    {           book_id::acts, "ACT"},
    {         book_id::romans, "ROM"},
    {   book_id::corinthians1, "1CO"},
    {   book_id::corinthians2, "2CO"},
    {      book_id::galatians, "GAL"},
    {      book_id::ephesians, "EPH"},
    {    book_id::philippians, "PHP"},
    {     book_id::colossians, "COL"},
    { book_id::thessalonians1, "1TH"},
    { book_id::thessalonians2, "2TH"},
    {       book_id::timothy1, "1TI"},
    {       book_id::timothy2, "2TI"},
    {          book_id::titus, "TIT"},
    {       book_id::philemon, "PHM"},
    {        book_id::hebrews, "HEB"},
    {          book_id::james, "JAS"},
    {         book_id::peter1, "1PE"},
    {         book_id::peter2, "2PE"},
    {          book_id::john1, "1JN"},
    {          book_id::john2, "2JN"},
    {          book_id::john3, "3JN"},
    {           book_id::jude, "JUD"},
    {     book_id::revelation, "REV"}
  });
  static_assert(books.size() == util::enum_count<book_id>());

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
  const std::unique_ptr<pugi::xml_document> book_data_;
};

} // namespace bibstd::bible
