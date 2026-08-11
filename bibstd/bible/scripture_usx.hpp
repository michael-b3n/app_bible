#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/scripture.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/util/enum.hpp"

#include <map>
#include <memory>
#include <optional>
#include <type_traits>

namespace bibstd::io
{
// Forward declarations
class zip_file_reader;
} // namespace bibstd::io
namespace bibstd::bible
{

///
/// Scripture scripture for USX formatted files.
///
class scripture_usx final : public scripture
{
  // Variables
  const info_type info_data_;
  const std::map<book_id, book_name_type> book_name_data_;
  const std::map<reference_type, passage_html_type> verse_data_;
  const versification_type versification_;

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

public: // Typedefs
  using passage_map_type = std::remove_const_t<decltype(verse_data_)>;
  using book_name_map_type = std::remove_const_t<decltype(book_name_data_)>;

public: // Creators
  ///
  /// Create a scripture_usx instance by loading and parsing USX files from the provided zip reader.
  /// \return A unique pointer to the created scripture_usx instance, or nullptr on failure
  ///
  static auto create(const io::zip_file_reader& zip_reader) -> std::unique_ptr<scripture>;

public: // Constructor
  scripture_usx(info_type info_data, book_name_map_type book_name_data, passage_map_type verse_data);
  ~scripture_usx() noexcept override;

private: // Overrides
  ///
  /// \see scripture::information
  ///
  auto do_information() const -> info_type override;

  ///
  /// The names originate from the header paragraphs of the book's own USX document.
  /// \see scripture::book_information
  ///
  auto do_book_information(book_id book) const -> std::optional<book_name_type> override;

  ///
  /// \see scripture::passage_html
  ///
  auto do_passage_html(const reference_type& ref) const -> std::optional<passage_html_type> override;

  ///
  /// \see scripture::versification
  ///
  auto do_versification() const -> const versification_type& override;
};

} // namespace bibstd::bible
