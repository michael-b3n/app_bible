#pragma once

#include "bible/common.hpp"
#include "util/const_bimap.hpp"
#include "util/enum.hpp"
#include "util/string.hpp"

#include <algorithm>
#include <array>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>

namespace bibstd::bible
{

///
/// Holds all german bible book name variants corresponding to book id.
///
class book_name_variants_de final
{
private: // Typedefs
  using book_name_type = std::pair<book_id, std::string_view>;

public: // Constants
  ///
  /// All german bible book name variants listed with corresponding book id.
  /// Whitespaces and fullstops are omitted.
  ///
  // clang-format off
  static constexpr auto name_variants = std::tuple{
    std::pair{        book_id::genesis,        util::to_array("1Mo", "1Mose", "Genesis", "Gen")},
    std::pair{         book_id::exodus,          util::to_array("2Mo", "2Mose", "Exodus", "Ex")},
    std::pair{      book_id::leviticus,      util::to_array("3Mo", "3Mose", "Levitikus", "Lev")},
    std::pair{        book_id::numbers,         util::to_array("4Mo", "4Mose", "Numeri", "Num")},
    std::pair{    book_id::deuteronomy,  util::to_array("5Mo", "5Mose", "Deuteronomium", "Dtn")},
    std::pair{         book_id::joshua,                          util::to_array("Josua", "Jos")},
    std::pair{         book_id::judges,                         util::to_array("Richter", "Ri")},
    std::pair{           book_id::ruth,                                   util::to_array("Rut")},
    std::pair{        book_id::samuel1,                       util::to_array("1Samuel", "1Sam")},
    std::pair{        book_id::samuel2,                       util::to_array("2Samuel", "2Sam")},
    std::pair{         book_id::kings1,                util::to_array("1Könige", "1Kön", "1Kö")},
    std::pair{         book_id::kings2,                util::to_array("2Könige", "2Kön", "2Kö")},
    std::pair{    book_id::chronicles1,                      util::to_array("1Chronik", "1Chr")},
    std::pair{    book_id::chronicles2,                      util::to_array("2Chronik", "2Chr")},
    std::pair{           book_id::ezra,                                  util::to_array("Esra")},
    std::pair{       book_id::nehemiah,                        util::to_array("Nehemia", "Neh")},
    std::pair{         book_id::esther,                          util::to_array("Ester", "Est")},
    std::pair{            book_id::job,                            util::to_array("Hiob", "Hi")},
    std::pair{         book_id::psalms,                           util::to_array("Psalm", "Ps")},
    std::pair{       book_id::proverbs,                        util::to_array("Sprüche", "Spr")},
    std::pair{   book_id::ecclesiastes,                      util::to_array("Prediger", "Pred")},
    std::pair{book_id::song_of_solomon,               util::to_array("Hohelied", "Hhld", "Hld")},
    std::pair{         book_id::isaiah,                         util::to_array("Jesaja", "Jes")},
    std::pair{       book_id::jeremiah,                        util::to_array("Jeremia", "Jer")},
    std::pair{   book_id::lamentations,                   util::to_array("Klagelieder", "Klgl")},
    std::pair{        book_id::ezekiel,                       util::to_array("Hesekiel", "Hes")},
    std::pair{         book_id::daniel,                         util::to_array("Daniel", "Dan")},
    std::pair{          book_id::hosea,                          util::to_array("Hosea", "Hos")},
    std::pair{           book_id::joel,                                  util::to_array("Joel")},
    std::pair{           book_id::amos,                            util::to_array("Amos", "Am")},
    std::pair{        book_id::obadiah,                         util::to_array("Obadja", "Obd")},
    std::pair{          book_id::jonah,                                  util::to_array("Jona")},
    std::pair{          book_id::micah,                           util::to_array("Micha", "Mi")},
    std::pair{          book_id::nahum,                          util::to_array("Nahum", "Nah")},
    std::pair{       book_id::habakkuk,                        util::to_array("Habakuk", "Hab")},
    std::pair{      book_id::zephaniah,                        util::to_array("Zefanja", "Zef")},
    std::pair{         book_id::haggai,                         util::to_array("Haggai", "Hag")},
    std::pair{      book_id::zechariah,                      util::to_array("Sacharja", "Sach")},
    std::pair{        book_id::malachi,                       util::to_array("Maleachi", "Mal")},
    std::pair{        book_id::matthew,                        util::to_array("Matthäus", "Mt")},
    std::pair{           book_id::mark,                          util::to_array("Markus", "Mk")},
    std::pair{           book_id::luke,                           util::to_array("Lukas", "Lk")},
    std::pair{           book_id::john,                       util::to_array("Johannes", "Joh")},
    std::pair{           book_id::acts,              util::to_array("Apostelgeschichte", "Apg")},
    std::pair{         book_id::romans,                          util::to_array("Römer", "Röm")},
    std::pair{   book_id::corinthians1,                    util::to_array("1Korinther", "1Kor")},
    std::pair{   book_id::corinthians2,                    util::to_array("2Korinther", "2Kor")},
    std::pair{      book_id::galatians,                        util::to_array("Galater", "Gal")},
    std::pair{      book_id::ephesians,                        util::to_array("Epheser", "Eph")},
    std::pair{    book_id::philippians,                     util::to_array("Philipper", "Phil")},
    std::pair{     book_id::colossians,                       util::to_array("Kolosser", "Kol")},
    std::pair{ book_id::thessalonians1,             util::to_array("1Thessalonicher", "1Thess")},
    std::pair{ book_id::thessalonians2,             util::to_array("2Thessalonicher", "2Thess")},
    std::pair{       book_id::timothy1,                    util::to_array("1Timotheus", "1Tim")},
    std::pair{       book_id::timothy2,                    util::to_array("2Timotheus", "2Tim")},
    std::pair{          book_id::titus,                          util::to_array("Titus", "Tit")},
    std::pair{       book_id::philemon,                      util::to_array("Philemon", "Phlm")},
    std::pair{        book_id::hebrews,                       util::to_array("Hebräer", "Hebr")},
    std::pair{          book_id::james,                        util::to_array("Jakobus", "Jak")},
    std::pair{         book_id::peter1,                      util::to_array("1Petrus", "1Petr")},
    std::pair{         book_id::peter2,                      util::to_array("2Petrus", "2Petr")},
    std::pair{          book_id::john1,                     util::to_array("1Johannes", "1Joh")},
    std::pair{          book_id::john2,                     util::to_array("2Johannes", "2Joh")},
    std::pair{          book_id::john3,                     util::to_array("3Johannes", "3Joh")},
    std::pair{           book_id::jude,                          util::to_array("Judas", "Jud")},
    std::pair{     book_id::revelation,         util::to_array("Offenbarung", "Offenb", "Offb")}
  };
  // clang-format on
  static_assert(std::tuple_size_v<decltype(name_variants)> == util::to_integral(book_id::END));

  // clang-format off
  static constexpr auto pretty_names = util::const_bimap(
    book_name_type{book_id::genesis,            "1.Mose"},
    book_name_type{book_id::exodus,             "2.Mose"},
    book_name_type{book_id::leviticus,          "3.Mose"},
    book_name_type{book_id::numbers,            "4.Mose"},
    book_name_type{book_id::deuteronomy,        "5.Mose"},
    book_name_type{book_id::joshua,             "Josua"},
    book_name_type{book_id::judges,             "Richter"},
    book_name_type{book_id::ruth,               "Rut"},
    book_name_type{book_id::samuel1,            "1.Samuel"},
    book_name_type{book_id::samuel2,            "2.Samuel"},
    book_name_type{book_id::kings1,             "1.Könige"},
    book_name_type{book_id::kings2,             "2.Könige"},
    book_name_type{book_id::chronicles1,        "1.Chronik"},
    book_name_type{book_id::chronicles2,        "2.Chronik"},
    book_name_type{book_id::ezra,               "Esra"},
    book_name_type{book_id::nehemiah,           "Nehemia"},
    book_name_type{book_id::esther,             "Ester"},
    book_name_type{book_id::job,                "Hiob"},
    book_name_type{book_id::psalms,             "Psalm"},
    book_name_type{book_id::proverbs,           "Sprüche"},
    book_name_type{book_id::ecclesiastes,       "Prediger"},
    book_name_type{book_id::song_of_solomon,    "Hoheslied"},
    book_name_type{book_id::isaiah,             "Jesaja"},
    book_name_type{book_id::jeremiah,           "Jeremia"},
    book_name_type{book_id::lamentations,       "Klagelieder"},
    book_name_type{book_id::ezekiel,            "Hesekiel"},
    book_name_type{book_id::daniel,             "Daniel"},
    book_name_type{book_id::hosea,              "Hosea"},
    book_name_type{book_id::joel,               "Joel"},
    book_name_type{book_id::amos,               "Amos"},
    book_name_type{book_id::obadiah,            "Obadja"},
    book_name_type{book_id::jonah,              "Jona"},
    book_name_type{book_id::micah,              "Micha"},
    book_name_type{book_id::nahum,              "Nahum"},
    book_name_type{book_id::habakkuk,           "Habakuk"},
    book_name_type{book_id::zephaniah,          "Zefanja"},
    book_name_type{book_id::haggai,             "Haggai"},
    book_name_type{book_id::zechariah,          "Sacharja"},
    book_name_type{book_id::malachi,            "Maleachi"},
    book_name_type{book_id::matthew,            "Matthäus"},
    book_name_type{book_id::mark,               "Markus"},
    book_name_type{book_id::luke,               "Lukas"},
    book_name_type{book_id::john,               "Johannes"},
    book_name_type{book_id::acts,               "Apostelgeschichte"},
    book_name_type{book_id::romans,             "Römer"},
    book_name_type{book_id::corinthians1,       "1.Korinther"},
    book_name_type{book_id::corinthians2,       "2.Korinther"},
    book_name_type{book_id::galatians,          "Galater"},
    book_name_type{book_id::ephesians,          "Epheser"},
    book_name_type{book_id::philippians,        "Philipper"},
    book_name_type{book_id::colossians,         "Kolosser"},
    book_name_type{book_id::thessalonians1,     "1.Thessalonicher"},
    book_name_type{book_id::thessalonians2,     "2.Thessalonicher"},
    book_name_type{book_id::timothy1,           "1.Timotheus"},
    book_name_type{book_id::timothy2,           "2.Timotheus"},
    book_name_type{book_id::titus,              "Titus"},
    book_name_type{book_id::philemon,           "Philemon"},
    book_name_type{book_id::hebrews,            "Hebräer"},
    book_name_type{book_id::james,              "Jakobus"},
    book_name_type{book_id::peter1,             "1.Petrus"},
    book_name_type{book_id::peter2,             "2.Petrus"},
    book_name_type{book_id::john1,              "1.Johannes"},
    book_name_type{book_id::john2,              "2.Johannes"},
    book_name_type{book_id::john3,              "3.Johannes"},
    book_name_type{book_id::jude,               "Judas"},
    book_name_type{book_id::revelation,         "Offenbarung"}
  );
  // clang-format on
  static_assert(pretty_names.size() == util::to_integral(book_id::END));

  ///
  /// Concatenated list of all bible book name variants.
  ///
  static constexpr auto name_variants_list = []()
  {
    constexpr auto get = [&]<std::size_t I>()
    {
      const auto element = std::get<I>(name_variants);
      constexpr auto size = std::tuple_size_v<decltype(element.second)>;
      std::array<std::pair<book_id, std::string_view>, size> result;
      std::ranges::for_each(
        std::views::iota(decltype(size){0}, size),
        [&](const auto i) { result.at(i) = std::pair{element.first, element.second.at(i)}; }
      );
      return result;
    };
    constexpr auto to_array = [](auto&& tuple)
    {
      constexpr auto get_array = [](auto&&... e) { return std::array{std::forward<decltype(e)>(e)...}; };
      return std::apply(get_array, std::forward<decltype(tuple)>(tuple));
    };
    return [&]<std::size_t... I>(std::index_sequence<I...>)
    {
      return to_array(std::tuple_cat(get.template operator()<I>()...));
    }(std::make_index_sequence<std::tuple_size_v<decltype(name_variants)>>{});
  }();
};

} // namespace bibstd::bible
