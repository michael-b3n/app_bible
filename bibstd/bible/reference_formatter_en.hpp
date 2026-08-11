#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/util/const_map.hpp"

#include <string_view>

namespace bibstd::bible
{

struct reference_formatter_en final
{
  // Constants
  static constexpr auto pretty_names = util::make_const_bimap<book_id, std::string_view>({
    {        book_id::genesis,         "Genesis"},
    {         book_id::exodus,          "Exodus"},
    {      book_id::leviticus,       "Leviticus"},
    {        book_id::numbers,         "Numbers"},
    {    book_id::deuteronomy,     "Deuteronomy"},
    {         book_id::joshua,          "Joshua"},
    {         book_id::judges,          "Judges"},
    {           book_id::ruth,            "Ruth"},
    {        book_id::samuel1,        "1 Samuel"},
    {        book_id::samuel2,        "2 Samuel"},
    {         book_id::kings1,         "1 Kings"},
    {         book_id::kings2,         "2 Kings"},
    {    book_id::chronicles1,    "1 Chronicles"},
    {    book_id::chronicles2,    "2 Chronicles"},
    {           book_id::ezra,            "Ezra"},
    {       book_id::nehemiah,        "Nehemiah"},
    {         book_id::esther,          "Esther"},
    {            book_id::job,             "Job"},
    {         book_id::psalms,          "Psalms"},
    {       book_id::proverbs,        "Proverbs"},
    {   book_id::ecclesiastes,    "Ecclesiastes"},
    {book_id::song_of_solomon, "Song of Solomon"},
    {         book_id::isaiah,          "Isaiah"},
    {       book_id::jeremiah,        "Jeremiah"},
    {   book_id::lamentations,    "Lamentations"},
    {        book_id::ezekiel,         "Ezekiel"},
    {         book_id::daniel,          "Daniel"},
    {          book_id::hosea,           "Hosea"},
    {           book_id::joel,            "Joel"},
    {           book_id::amos,            "Amos"},
    {        book_id::obadiah,         "Obadiah"},
    {          book_id::jonah,           "Jonah"},
    {          book_id::micah,           "Micah"},
    {          book_id::nahum,           "Nahum"},
    {       book_id::habakkuk,        "Habakkuk"},
    {      book_id::zephaniah,       "Zephaniah"},
    {         book_id::haggai,          "Haggai"},
    {      book_id::zechariah,       "Zechariah"},
    {        book_id::malachi,         "Malachi"},
    {        book_id::matthew,         "Matthew"},
    {           book_id::mark,            "Mark"},
    {           book_id::luke,            "Luke"},
    {           book_id::john,            "John"},
    {           book_id::acts,            "Acts"},
    {         book_id::romans,          "Romans"},
    {   book_id::corinthians1,   "1 Corinthians"},
    {   book_id::corinthians2,   "2 Corinthians"},
    {      book_id::galatians,       "Galatians"},
    {      book_id::ephesians,       "Ephesians"},
    {    book_id::philippians,     "Philippians"},
    {     book_id::colossians,      "Colossians"},
    { book_id::thessalonians1, "1 Thessalonians"},
    { book_id::thessalonians2, "2 Thessalonians"},
    {       book_id::timothy1,       "1 Timothy"},
    {       book_id::timothy2,       "2 Timothy"},
    {          book_id::titus,           "Titus"},
    {       book_id::philemon,        "Philemon"},
    {        book_id::hebrews,         "Hebrews"},
    {          book_id::james,           "James"},
    {         book_id::peter1,         "1 Peter"},
    {         book_id::peter2,         "2 Peter"},
    {          book_id::john1,          "1 John"},
    {          book_id::john2,          "2 John"},
    {          book_id::john3,          "3 John"},
    {           book_id::jude,            "Jude"},
    {     book_id::revelation,      "Revelation"}
  });
  static_assert(pretty_names.size() == util::enum_count<book_id>());
};

} // namespace bibstd::bible
