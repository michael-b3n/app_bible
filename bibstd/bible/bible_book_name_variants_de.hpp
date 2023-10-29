#pragma once

#include "bible/common.hpp"
#include "util/string_view.hpp"

#include <array>
#include <tuple>
#include <utility>

namespace bibstd::bible
{

///
/// Holds all german bible book name variants corresponding to book id.
///
struct bible_book_name_variants_de final
{
  // Constants
  ///
  /// All german bible book name variants listed with corresponding book id.
  ///
  static constexpr auto name_variants = std::tuple{
  // clang-format off
    std::pair{        book::genesis,                        util::to_array("1.Mose", "1Mose", "Genesis", "Gen")},
    std::pair{         book::exodus,                          util::to_array("2.Mose", "2Mose", "Exodus", "Ex")},
    std::pair{      book::leviticus,                      util::to_array("3.Mose", "3Mose", "Levitikus", "Lev")},
    std::pair{        book::numbers,                         util::to_array("4.Mose", "4Mose", "Numeri", "Num")},
    std::pair{    book::deuteronomy,                  util::to_array("5.Mose", "5Mose", "Deuteronomium", "Dtn")},
    std::pair{         book::joshua,                                             util::to_array("Josua", "Jos")},
    std::pair{         book::judges,                                            util::to_array("Richter", "Ri")},
    std::pair{           book::ruth,                                                      util::to_array("Rut")},
    std::pair{        book::samuel1,                     util::to_array("1.Samuel", "1Samuel", "1.Sam", "1Sam")},
    std::pair{        book::samuel2,                     util::to_array("2.Samuel", "2Samuel", "2.Sam", "2Sam")},
    std::pair{         book::kings1,                     util::to_array("1.Könige", "1Könige", "1.Kön", "1Kön")},
    std::pair{         book::kings2,                     util::to_array("2.Könige", "2Könige", "2.Kön", "2Kön")},
    std::pair{    book::chronicles1,                   util::to_array("1.Chronik", "1Chronik", "1.Chr", "1Chr")},
    std::pair{    book::chronicles2,                   util::to_array("2.Chronik", "2Chronik", "2.Chr", "2Chr")},
    std::pair{           book::ezra,                                                     util::to_array("Esra")},
    std::pair{       book::nehemiah,                                           util::to_array("Nehemia", "Neh")},
    std::pair{         book::esther,                                             util::to_array("Ester", "Est")},
    std::pair{            book::job,                                               util::to_array("Hiob", "Hi")},
    std::pair{         book::psalms,                                              util::to_array("Psalm", "Ps")},
    std::pair{       book::proverbs,                                           util::to_array("Sprüche", "Spr")},
    std::pair{   book::ecclesiastes,                                         util::to_array("Prediger", "Pred")},
    std::pair{book::song_of_solomon,                                          util::to_array("Hohelied", "Hld")},
    std::pair{         book::isaiah,                                            util::to_array("Jesaja", "Jes")},
    std::pair{       book::jeremiah,                                           util::to_array("Jeremia", "Jer")},
    std::pair{   book::lamentations,                                      util::to_array("Klagelieder", "Klgl")},
    std::pair{        book::ezekiel,                                          util::to_array("Hesekiel", "Hes")},
    std::pair{         book::daniel,                                            util::to_array("Daniel", "Dan")},
    std::pair{          book::hosea,                                             util::to_array("Hosea", "Hos")},
    std::pair{           book::joel,                                                     util::to_array("Joel")},
    std::pair{           book::amos,                                               util::to_array("Amos", "Am")},
    std::pair{        book::obadiah,                                            util::to_array("Obadja", "Obd")},
    std::pair{          book::jonah,                                                     util::to_array("Jona")},
    std::pair{          book::micah,                                              util::to_array("Micha", "Mi")},
    std::pair{          book::nahum,                                             util::to_array("Nahum", "Nah")},
    std::pair{       book::habbakuk,                                           util::to_array("Habakuk", "Hab")},
    std::pair{      book::zephaniah,                                           util::to_array("Zefanja", "Zef")},
    std::pair{         book::haggai,                                            util::to_array("Haggai", "Hag")},
    std::pair{      book::zechariah,                                         util::to_array("Sacharja", "Sach")},
    std::pair{        book::malachi,                                          util::to_array("Maleachi", "Mal")},
    std::pair{        book::matthew,                                           util::to_array("Matthäus", "Mt")},
    std::pair{           book::mark,                                             util::to_array("Markus", "Mk")},
    std::pair{           book::luke,                                              util::to_array("Lukas", "Lk")},
    std::pair{           book::john,                                          util::to_array("Johannes", "Joh")},
    std::pair{           book::acts,                                 util::to_array("Apostelgeschichte", "Apg")},
    std::pair{         book::romans,                                             util::to_array("Römer", "Röm")},
    std::pair{   book::corinthians1,               util::to_array("1.Korinther", "1Korinther", "1.Kor", "1Kor")},
    std::pair{   book::corinthians2,               util::to_array("2.Korinther", "2Korinther", "2.Kor", "2Kor")},
    std::pair{      book::galatians,                                           util::to_array("Galater", "Gal")},
    std::pair{      book::ephesians,                                           util::to_array("Epheser", "Eph")},
    std::pair{    book::philippians,                                        util::to_array("Philipper", "Phil")},
    std::pair{     book::colossians,                                          util::to_array("Kolosser", "Kol")},
    std::pair{ book::thessalonians1, util::to_array("1.Thessalonicher", "1Thessalonicher", "1.Thess", "1Thess")},
    std::pair{ book::thessalonians2, util::to_array("2.Thessalonicher", "2Thessalonicher", "2.Thess", "2Thess")},
    std::pair{       book::timothy1,               util::to_array("1.Timotheus", "1Timotheus", "1.Tim", "1Tim")},
    std::pair{       book::timothy2,               util::to_array("2.Timotheus", "2Timotheus", "2.Tim", "2Tim")},
    std::pair{          book::titus,                                             util::to_array("Titus", "Tit")},
    std::pair{       book::philemon,                                         util::to_array("Philemon", "Phlm")},
    std::pair{        book::hebrews,                                          util::to_array("Hebräer", "Hebr")},
    std::pair{          book::james,                                           util::to_array("Jakobus", "Jak")},
    std::pair{         book::peter1,                   util::to_array("1.Petrus", "1Petrus", "1.Petr", "1Petr")},
    std::pair{         book::peter2,                   util::to_array("2.Petrus", "2Petrus", "2.Petr", "2Petr")},
    std::pair{          book::john1,                 util::to_array("1.Johannes", "1Johannes", "1.Joh", "1Joh")},
    std::pair{          book::john2,                 util::to_array("2.Johannes", "2Johannes", "2.Joh", "2Joh")},
    std::pair{          book::john3,                 util::to_array("3.Johannes", "3Johannes", "3.Joh", "3Joh")},
    std::pair{           book::jude,                                             util::to_array("Judas", "Jud")},
    std::pair{     book::revelation,                                      util::to_array("Offenbarung", "Offb")}
  // clang-format on
  };

  ///
  /// Concatenated list of all bible book name variants.
  ///
  static constexpr auto name_variants_list = []()
  {
    constexpr auto get = [&]<std::size_t I>() { return std::get<I>(name_variants).second; };
    constexpr auto to_array = [](auto&& tuple)
    {
      constexpr auto get_array = [](auto&&... e) { return std::array{std::forward<decltype(e)>(e)...}; };
      return std::apply(get_array, std::forward<decltype(tuple)>(tuple));
    };
    return [&]<std::size_t... I>(std::index_sequence<I...>)
    { return to_array(std::tuple_cat(get.template operator()<I>()...)); }(std::make_index_sequence<std::tuple_size_v<decltype(name_variants)>>{});
  }();
};

} // namespace bibstd::bible
