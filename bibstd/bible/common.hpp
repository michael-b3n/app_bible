#pragma once

#include "bibstd/util/enum.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>

namespace bibstd::bible
{

///
/// Bible books.
///
enum class book_id
{
  genesis,
  exodus,
  leviticus,
  numbers,
  deuteronomy,
  joshua,
  judges,
  ruth,
  samuel1,
  samuel2,
  kings1,
  kings2,
  chronicles1,
  chronicles2,
  ezra,
  nehemiah,
  esther,
  job,
  psalms,
  proverbs,
  ecclesiastes,
  song_of_solomon,
  isaiah,
  jeremiah,
  lamentations,
  ezekiel,
  daniel,
  hosea,
  joel,
  amos,
  obadiah,
  jonah,
  micah,
  nahum,
  habakkuk,
  zephaniah,
  haggai,
  zechariah,
  malachi,
  matthew,
  mark,
  luke,
  john,
  acts,
  romans,
  corinthians1,
  corinthians2,
  galatians,
  ephesians,
  philippians,
  colossians,
  thessalonians1,
  thessalonians2,
  timothy1,
  timothy2,
  titus,
  philemon,
  hebrews,
  james,
  peter1,
  peter2,
  john1,
  john2,
  john3,
  jude,
  revelation
};

static_assert(util::enum_count<book_id>() == 66); // bible has 66 books
static_assert(std::ranges::none_of(util::enum_names<book_id>(), [](const auto name) { return name.contains('-'); }));

///
/// Bible testaments.
///
enum class testament_id
{
  ot,
  nt
};

///
/// Bible translations.
///
enum class translation
{
  dbu,
  elb,
  esv,
  eu,
  gnb,
  hfa,
  kjv,
  lut,
  meng,
  neu,
  ngu,
  nirv,
  niv,
  nlb,
  slt,
  vxb,
  zb
};

///
/// Number of all the verses in the bible.
///
constexpr auto total_verse_count = std::uint32_t{31102};

///
/// Get the count of chapters in a book.
/// \param book The book to get the chapter count of
/// \return the chapter count
///
auto chapter_count(book_id book) -> std::uint32_t;

///
/// Get the count of verses in a chapter.
/// \param book The book to get the chapter count of
/// \param chapter_number The chapter to get the verse count of
/// \return the verse count
///
auto verse_count(book_id book, std::uint32_t chapter_number) -> std::optional<std::uint32_t>;

} // namespace bibstd::bible

///
///
template<>
struct std::formatter<bibstd::bible::book_id> : std::formatter<std::string>
{
  auto format(const bibstd::bible::book_id e, std::format_context& ctx) const
  {
    return formatter<std::string>::format(std::format("{}", bibstd::util::enum_name(e)), ctx);
  }
};
