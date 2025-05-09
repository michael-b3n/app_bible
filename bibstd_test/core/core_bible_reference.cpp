#include <core/core_bible_reference.hpp>
#include <util/contains.hpp>

#include <catch2/catch_all.hpp>

namespace bibstd::core
{

TEST_CASE("reference_parser", "[bible]")
{
  core_bible_reference core;
  const auto ref_genesis_1_1 = bible::reference::create(bible::book_id::genesis, 1u, 1u).value();
  const auto ref_genesis_1_2 = bible::reference::create(bible::book_id::genesis, 1u, 2u).value();
  const auto genesis_1_1 = bible::reference_range(ref_genesis_1_1);
  const auto genesis_1_1to2 = bible::reference_range(ref_genesis_1_1, ref_genesis_1_2);

  const auto ref_exodus_2_2 = bible::reference::create(bible::book_id::exodus, 2u, 2u).value();
  const auto ref_exodus_2_3 = bible::reference::create(bible::book_id::exodus, 2u, 3u).value();
  const auto ref_exodus_2_7 = bible::reference::create(bible::book_id::exodus, 2u, 7u).value();
  const auto ref_exodus_4_5 = bible::reference::create(bible::book_id::exodus, 4u, 5u).value();
  const auto ref_exodus_5_6 = bible::reference::create(bible::book_id::exodus, 5u, 6u).value();
  const auto ref_exodus_7_8 = bible::reference::create(bible::book_id::exodus, 7u, 8u).value();
  const auto exodus_2_2to3 = bible::reference_range(ref_exodus_2_2, ref_exodus_2_3);
  const auto exodus_2_7 = bible::reference_range(ref_exodus_2_7);
  const auto exodus_4_5 = bible::reference_range(ref_exodus_4_5);
  const auto exodus_5_6to7_8 = bible::reference_range(ref_exodus_5_6, ref_exodus_7_8);

  const auto filler_text = std::string{"Lorem ipsum dolor sit"};

  CHECK(core.parse(filler_text, 0).ranges.empty());
  {
    const auto reference = std::string{"1.Mose 1:1"};
    const auto result = core.parse(reference, 0).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, genesis_1_1));
  }
  {
    const auto reference = std::string{"1.Mose 1;1-2"};
    const auto result = core.parse(reference, 0).ranges;
    CHECK(result.size() == 1);
    CHECK(util::contains(result, genesis_1_1to2));
  }
  {
    const auto reference = std::string{"2.Mose 2,2-3.7;4,5;5,6-7,8"};
    const auto result = core.parse(reference, 0).ranges;
    CHECK(result.size() == 4);
    CHECK(util::contains(result, exodus_2_2to3));
    CHECK(util::contains(result, exodus_2_7));
    CHECK(util::contains(result, exodus_4_5));
    CHECK(util::contains(result, exodus_5_6to7_8));
  }
}

} // namespace bibstd::core
