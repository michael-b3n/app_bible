#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/util/const_map.hpp"

#include <string>
#include <string_view>

namespace bibstd::bible
{

struct reference_formatter_de final
{
  // Constants
  static constexpr auto pretty_names = util::make_const_bimap<book_id, std::string_view>({
    {        book_id::genesis,            "1.Mose"},
    {         book_id::exodus,            "2.Mose"},
    {      book_id::leviticus,            "3.Mose"},
    {        book_id::numbers,            "4.Mose"},
    {    book_id::deuteronomy,            "5.Mose"},
    {         book_id::joshua,             "Josua"},
    {         book_id::judges,           "Richter"},
    {           book_id::ruth,               "Rut"},
    {        book_id::samuel1,          "1.Samuel"},
    {        book_id::samuel2,          "2.Samuel"},
    {         book_id::kings1,          "1.Könige"},
    {         book_id::kings2,          "2.Könige"},
    {    book_id::chronicles1,         "1.Chronik"},
    {    book_id::chronicles2,         "2.Chronik"},
    {           book_id::ezra,              "Esra"},
    {       book_id::nehemiah,           "Nehemia"},
    {         book_id::esther,             "Ester"},
    {            book_id::job,              "Hiob"},
    {         book_id::psalms,             "Psalm"},
    {       book_id::proverbs,           "Sprüche"},
    {   book_id::ecclesiastes,          "Prediger"},
    {book_id::song_of_solomon,         "Hoheslied"},
    {         book_id::isaiah,            "Jesaja"},
    {       book_id::jeremiah,           "Jeremia"},
    {   book_id::lamentations,       "Klagelieder"},
    {        book_id::ezekiel,          "Hesekiel"},
    {         book_id::daniel,            "Daniel"},
    {          book_id::hosea,             "Hosea"},
    {           book_id::joel,              "Joel"},
    {           book_id::amos,              "Amos"},
    {        book_id::obadiah,            "Obadja"},
    {          book_id::jonah,              "Jona"},
    {          book_id::micah,             "Micha"},
    {          book_id::nahum,             "Nahum"},
    {       book_id::habakkuk,           "Habakuk"},
    {      book_id::zephaniah,           "Zefanja"},
    {         book_id::haggai,            "Haggai"},
    {      book_id::zechariah,          "Sacharja"},
    {        book_id::malachi,          "Maleachi"},
    {        book_id::matthew,          "Matthäus"},
    {           book_id::mark,            "Markus"},
    {           book_id::luke,             "Lukas"},
    {           book_id::john,          "Johannes"},
    {           book_id::acts, "Apostelgeschichte"},
    {         book_id::romans,             "Römer"},
    {   book_id::corinthians1,       "1.Korinther"},
    {   book_id::corinthians2,       "2.Korinther"},
    {      book_id::galatians,           "Galater"},
    {      book_id::ephesians,           "Epheser"},
    {    book_id::philippians,         "Philipper"},
    {     book_id::colossians,          "Kolosser"},
    { book_id::thessalonians1,  "1.Thessalonicher"},
    { book_id::thessalonians2,  "2.Thessalonicher"},
    {       book_id::timothy1,       "1.Timotheus"},
    {       book_id::timothy2,       "2.Timotheus"},
    {          book_id::titus,             "Titus"},
    {       book_id::philemon,          "Philemon"},
    {        book_id::hebrews,           "Hebräer"},
    {          book_id::james,           "Jakobus"},
    {         book_id::peter1,          "1.Petrus"},
    {         book_id::peter2,          "2.Petrus"},
    {          book_id::john1,        "1.Johannes"},
    {          book_id::john2,        "2.Johannes"},
    {          book_id::john3,        "3.Johannes"},
    {           book_id::jude,             "Judas"},
    {     book_id::revelation,       "Offenbarung"}
  });
  static_assert(pretty_names.size() == util::enum_count<book_id>());

  // Operators
  ///
  /// Formats a reference range to pretty format.
  /// \param range Reference range that shall be formatted
  /// \return formatted reference range as string
  ///
  static auto operator()(const reference_range& range) -> std::string;
};

} // namespace bibstd::bible
