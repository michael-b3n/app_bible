#include <bibstd/bible/reference_parser.hpp>
#include <bibstd/bible/scripture.hpp>
#include <bibstd/util/contains.hpp>

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace bibstd::bible
{
namespace
{

constexpr auto esv = versification_esv;

///
/// Parse the text with the german language and the ESV versification.
/// \return Parse result
///
auto parse(const std::string_view text, const std::size_t index) -> reference_parser::parse_result
{
  return reference_parser::parse(text, index, util::language::german, esv);
}

///
/// \return Reference range covering exactly one verse
///
auto verse(const book_id book, const std::uint32_t chapter, const std::uint32_t number) -> reference_range
{
  return reference_range{reference::create_unguarded(book, chapter, number)};
}

///
/// \return Reference range spanning from the first to the second verse
///
auto verses(
  const book_id book,
  const std::uint32_t chapter_begin,
  const std::uint32_t verse_begin,
  const std::uint32_t chapter_end,
  const std::uint32_t verse_end
) -> reference_range
{
  return reference_range{
    reference::create_unguarded(book, chapter_begin, verse_begin), reference::create_unguarded(book, chapter_end, verse_end)
  };
}

///
/// \return Substring of the text covered by index_range_origin
///
auto origin(const std::string_view text, const reference_parser::parse_result& result) -> std::string_view
{
  const auto& range = result.index_range_origin;
  return text.substr(range.begin, range.end - range.begin);
}

} // namespace

TEST_CASE("reference_parser parses a single reference", "[bible]")
{
  {
    const auto text = std::string_view{"1.Mose 1:1"};
    const auto result = parse(text, 0).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::genesis, 1, 1)));
  }
  {
    const auto text = std::string_view{"1.Mose 1;1-2"};
    const auto result = parse(text, 0).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verses(book_id::genesis, 1, 1, 1, 2)));
  }
  {
    const auto text = std::string_view{"2.Mose 2,2-3.7;4,5;5,6-7,8"};
    const auto result = parse(text, 0).ranges;
    CHECK(result.size() == 4);
    CHECK(util::contains(result, verses(book_id::exodus, 2, 2, 2, 3)));
    CHECK(util::contains(result, verse(book_id::exodus, 2, 7)));
    CHECK(util::contains(result, verse(book_id::exodus, 4, 5)));
    CHECK(util::contains(result, verses(book_id::exodus, 5, 6, 7, 8)));
  }
}

TEST_CASE("reference_parser accepts every transition character", "[bible]")
{
  // Every character of `transition_chars` separates a chapter from a verse or a verse from a verse.
  for(const auto separator : std::string_view{",;:"})
  {
    const auto text = std::string{"Joh 3"} + separator + "16";
    INFO("text: " << text);
    const auto result = parse(text, 0).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
  }
  for(const auto separator : std::string_view{".&+;"})
  {
    const auto text = std::string{"Joh 3,16"} + separator + "18";
    INFO("text: " << text);
    const auto result = parse(text, 0).ranges;
    CHECK(result.size() == 2);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
    CHECK(util::contains(result, verse(book_id::john, 3, 18)));
  }
}

TEST_CASE("reference_parser expands a reference without a verse", "[bible]")
{
  {
    // A bare chapter covers the whole chapter.
    const auto result = parse("1.Mose 5", 0).ranges;
    CHECK(result.size() == 1);
    CHECK(
      util::contains(result, verses(book_id::genesis, 5, 1, 5, esv.verse_count(book_id::genesis, reference::chapter_type{5})))
    );
  }
  {
    // A bare chapter range covers everything from the first to the last chapter.
    const auto result = parse("1.Mose 1-2", 0).ranges;
    CHECK(result.size() == 1);
    CHECK(
      util::contains(result, verses(book_id::genesis, 1, 1, 2, esv.verse_count(book_id::genesis, reference::chapter_type{2})))
    );
  }
}

TEST_CASE("reference_parser ignores number postfixes", "[bible]")
{
  for(const auto postfix : std::vector<std::string_view>{"ff.", "f.", "a", "b", "c", "d"})
  {
    const auto text = std::string{"Joh 3,16"} + std::string{postfix};
    INFO("text: " << text);
    const auto result = parse(text, 0).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
  }
}

TEST_CASE("reference_parser resolves the reference under the cursor", "[bible]")
{
  const auto text = std::string_view{"1.Mose 1,1 und 2.Mose 2,2"};
  {
    // Cursor on the first book name.
    const auto result = parse(text, 2);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verse(book_id::genesis, 1, 1)));
    CHECK(origin(text, result) == "1.Mose 1,1");
  }
  {
    // Cursor on the numbers of the second reference.
    const auto result = parse(text, 22);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verse(book_id::exodus, 2, 2)));
    CHECK(origin(text, result) == "2.Mose 2,2");
  }
  {
    // Cursor on the filler word between both references.
    const auto result = parse(text, 12);
    CHECK(result.ranges.empty());
  }
}

TEST_CASE("reference_parser does not confuse a book with its numbered variant", "[bible]")
{
  {
    const auto result = parse("Joh 1,1", 1).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::john, 1, 1)));
  }
  {
    const auto result = parse("1.Joh 1,1", 3).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::john1, 1, 1)));
  }
}

TEST_CASE("reference_parser does not take numbers of a following book", "[bible]")
{
  // The "1" in front of "1.Mose" starts the next reference and must not extend the first one.
  const auto text = std::string_view{"Joh 3,16 1.Mose 1,1"};
  const auto result = parse(text, 1);
  CHECK(result.ranges.size() == 1);
  CHECK(util::contains(result.ranges, verse(book_id::john, 3, 16)));
  CHECK(origin(text, result) == "Joh 3,16");
}

TEST_CASE("reference_parser rejects references outside the versification", "[bible]")
{
  CHECK(parse("1.Mose 99,1", 2).ranges.empty());
  CHECK(parse("1.Mose 1,99", 2).ranges.empty());
}

TEST_CASE("reference_parser handles degenerate input", "[bible]")
{
  CHECK(parse("", 0).ranges.empty());
  CHECK(parse("Lorem ipsum dolor sit", 0).ranges.empty());

  // An index past the end of the text must not read out of bounds.
  CHECK(parse("1.Mose 1,1", 10).ranges.empty());
  CHECK(parse("1.Mose 1,1", 1000).ranges.empty());

  // A book name without any number is not a reference.
  CHECK(parse("1.Mose Und", 2).ranges.empty());
  CHECK(parse("steht in 1.Mose", 12).ranges.empty());
}

//
// The OCR hands `parse` a whole paragraph, so a single reference regularly straddles a line break.
// The line feed itself is dropped during normalization, which keeps the cases below working.
//

TEST_CASE("reference_parser parses a reference split by a line break", "[bible]")
{
  {
    // Line break between book name and numbers.
    const auto text = std::string_view{"1.Mose\n1,1"};
    const auto result = parse(text, 2);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verse(book_id::genesis, 1, 1)));
    CHECK(origin(text, result) == text);
  }
  {
    // Line break inside the numbers, directly after the transition character.
    const auto text = std::string_view{"Joh 3,\n16"};
    const auto result = parse(text, 1);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verse(book_id::john, 3, 16)));
  }
  {
    // Hyphenated book name at the end of a line.
    const auto text = std::string_view{"1.Mo-\nse 1,1 Und"};
    const auto result = parse(text, 2);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verse(book_id::genesis, 1, 1)));
  }
  {
    // Reference range continued on the next line.
    const auto text = std::string_view{"Joh 3,16-\n4,2"};
    const auto result = parse(text, 1);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verses(book_id::john, 3, 16, 4, 2)));
  }
  {
    // A word on the next line ends the reference.
    const auto text = std::string_view{"Der Vers steht in Joh 3,16\nund wurde oft zitiert"};
    const auto result = parse(text, 20);
    CHECK(result.ranges.size() == 1);
    CHECK(util::contains(result.ranges, verse(book_id::john, 3, 16)));
    CHECK(origin(text, result) == "Joh 3,16");
  }
}

//
// The numbers range reported by find_book reaches past the reference whenever more digits follow it,
// so a gap -- whitespace, a line break, or a word -- has to end the reference. It may only be
// crossed while a transition char is still waiting for its number.
//

TEST_CASE("reference_parser reads a reference spaced out around its separators", "[bible]")
{
  // Which character separates the numbers is only decided later by match_passage_template, so a gap
  // has to be crossed for every transition character, no matter which one it turns out to be. The
  // full stop is the exception, \see the sentence punctuation test below.
  for(const auto& text :
      std::vector<std::string_view>{"Joh 3, 16.18", "Joh 3,16 .18", "Joh 3,16 ;18", "Joh 3,16; 18", "Joh 3,16\n.18"})
  {
    INFO("text: " << text);
    const auto result = parse(text, 1).ranges;
    CHECK(result.size() == 2);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
    CHECK(util::contains(result, verse(book_id::john, 3, 18)));
  }
  for(const auto& text : std::vector<std::string_view>{"Joh 3,16-18", "Joh 3,16 -18", "Joh 3,16 - 18"})
  {
    INFO("text: " << text);
    const auto result = parse(text, 1).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verses(book_id::john, 3, 16, 3, 18)));
  }
  {
    // A spaced out chapter range must not collapse to its first chapter.
    const auto result = parse("1.Mose 1 - 2", 2).ranges;
    CHECK(result.size() == 1);
    CHECK(
      util::contains(result, verses(book_id::genesis, 1, 1, 2, esv.verse_count(book_id::genesis, reference::chapter_type{2})))
    );
  }
  {
    // A spaced out verse list followed by an unrelated number on the next line keeps the whole list.
    const auto result = parse("Joh 3, 16.18\n12 Personen folgten", 1).ranges;
    CHECK(result.size() == 2);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
    CHECK(util::contains(result, verse(book_id::john, 3, 18)));
  }
}

TEST_CASE("reference_parser does not absorb an unrelated number from the next line", "[bible]")
{
  const auto text = std::string_view{"Wie in Johannes 3,16\n12 Junger folgten ihm"};
  const auto result = parse(text, 9);
  CHECK(result.ranges.size() == 1);
  CHECK(util::contains(result.ranges, verse(book_id::john, 3, 16)));
  CHECK(origin(text, result) == "Johannes 3,16");
}

TEST_CASE("reference_parser does not shift a verse across a line break", "[bible]")
{
  // The absorbed number stays inside the versification here, so the reference is not rejected but
  // silently answered with 1.Mose 1,12.
  const auto result = parse("1.Mose 1,1\n2 Und Gott sprach", 2).ranges;
  CHECK(result.size() == 1);
  CHECK(util::contains(result, verse(book_id::genesis, 1, 1)));
}

TEST_CASE("reference_parser does not glue numbers separated by a word", "[bible]")
{
  // The word consists only of postfix letters, so it does not end the numbers range either.
  const auto result = parse("1.Mose 1,1 ab 2,3", 2).ranges;
  CHECK(result.size() == 1);
  CHECK(util::contains(result, verse(book_id::genesis, 1, 1)));
}

TEST_CASE("reference_parser keeps a reference with a postfix intact", "[bible]")
{
  // The stripped "ff." must not let the next line's number attach to the verse.
  const auto result = parse("Joh 3,16ff.\n5 Personen waren dabei", 1).ranges;
  CHECK(result.size() == 1);
  CHECK(util::contains(result, verse(book_id::john, 3, 16)));
}

TEST_CASE("reference_parser does not invent a reference for an unparsable number", "[bible]")
{
  // A number too large for std::uint32_t leaves the passage template empty, which is no reference.
  CHECK(parse("1.Mose 99999999999,1", 2).ranges.empty());
}

TEST_CASE("reference_parser does not read a sentence-final number as a verse", "[bible]")
{
  // A full stop followed by a gap ends a sentence, so the number opening the next one is no verse.
  {
    const auto result = parse("siehe Joh 3,16.\n5 Personen waren dabei", 7).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
  }
  {
    const auto result = parse("1.Mose 1,1. 5 Leute waren dabei", 2).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::genesis, 1, 1)));
  }
  {
    // The full stop still separates passage numbers when it is written without a gap.
    const auto result = parse("Joh 3,16.18", 1).ranges;
    CHECK(result.size() == 2);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
    CHECK(util::contains(result, verse(book_id::john, 3, 18)));
  }
  {
    // Accepted trade-off: a verse list written with a gap after the full stop loses its tail,
    // because it cannot be told apart from a sentence boundary.
    const auto result = parse("Joh 3,16. 18", 1).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, verse(book_id::john, 3, 16)));
  }
}

} // namespace bibstd::bible
