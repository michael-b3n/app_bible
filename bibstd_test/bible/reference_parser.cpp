#include <bible/reference_parser.hpp>
#include <util/contains.hpp>

#include <catch2/catch_all.hpp>

namespace bibstd::bible
{

TEST_CASE("reference_parser", "[bible]")
{
  reference_parser parser;
  const auto ref_genesis_1_1 = reference::create(book_id::genesis, 1u, 1u).value();
  const auto ref_genesis_1_2 = reference::create(book_id::genesis, 1u, 2u).value();
  const auto genesis_1_1 = reference_range(ref_genesis_1_1);
  const auto genesis_1_1to2 = reference_range(ref_genesis_1_1, ref_genesis_1_2);

  const auto ref_exodus_2_2 = reference::create(book_id::exodus, 2u, 2u).value();
  const auto ref_exodus_2_3 = reference::create(book_id::exodus, 2u, 3u).value();
  const auto ref_exodus_2_7 = reference::create(book_id::exodus, 2u, 7u).value();
  const auto ref_exodus_4_5 = reference::create(book_id::exodus, 4u, 5u).value();
  const auto ref_exodus_5_6 = reference::create(book_id::exodus, 5u, 6u).value();
  const auto ref_exodus_7_8 = reference::create(book_id::exodus, 7u, 8u).value();
  const auto exodus_2_2to3 = reference_range(ref_exodus_2_2, ref_exodus_2_3);
  const auto exodus_2_7 = reference_range(ref_exodus_2_7);
  const auto exodus_4_5 = reference_range(ref_exodus_4_5);
  const auto exodus_5_6to7_8 = reference_range(ref_exodus_5_6, ref_exodus_7_8);

  const auto filler_text = std::string{"Lorem ipsum dolor sit"};

  CHECK(parser.parse(filler_text).empty());
  {
    const auto reference = std::string{"1.Mose 1:1"};
    const auto result = parser.parse(reference);
    CHECK(result.size() == 1);
    CHECK(util::contains(result, genesis_1_1));
  }
  {
    const auto reference = std::string{"1.Mose 1;1-2"};
    const auto result = parser.parse(reference);
    CHECK(result.size() == 1);
    CHECK(util::contains(result, genesis_1_1to2));
  }
  {
    const auto reference = std::string{"2.Mose 2,2-3.7;4,5;5,6-7,8"};
    const auto result = parser.parse(reference);
    CHECK(result.size() == 4);
    CHECK(util::contains(result, exodus_2_2to3));
    CHECK(util::contains(result, exodus_2_7));
    CHECK(util::contains(result, exodus_4_5));
    CHECK(util::contains(result, exodus_5_6to7_8));
  }
}

} // namespace bibstd::bible
