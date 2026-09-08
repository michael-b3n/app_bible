#include <bibstd/bible/reference.hpp>
#include <bibstd/bible/versification.hpp>

#include <catch2/catch_test_macros.hpp>

#include <format>

namespace bibstd::bible
{

TEST_CASE("reference create_unguarded", "[bible]")
{
  GIVEN("chapter and verse as typesafe numbers")
  {
    constexpr auto ref = reference::create_unguarded(book_id::john, reference::chapter_type{3}, reference::verse_type{16});
    static_assert(ref.book() == book_id::john);
    static_assert(ref.chapter() == reference::chapter_type{3});
    static_assert(ref.verse() == reference::verse_type{16});
  }
  GIVEN("chapter and verse as plain integers")
  {
    static_assert(reference::create_unguarded(book_id::john, 3, 16) == reference::create_unguarded(book_id::john, 3, 16));
  }
  GIVEN("numbers outside the versification")
  {
    // create_unguarded does not validate, so an impossible reference is created as requested.
    constexpr auto ref = reference::create_unguarded(book_id::genesis, 99, 99);
    static_assert(ref.chapter() == reference::chapter_type{99});
    static_assert(!versification_esv.contains(ref));
  }
}

TEST_CASE("reference create", "[bible]")
{
  constexpr auto esv = versification_esv;
  GIVEN("a reference inside the versification")
  {
    CHECK(reference::create(book_id::genesis, 1, 1, esv) == reference::create_unguarded(book_id::genesis, 1, 1));
  }
  GIVEN("a chapter outside the versification")
  {
    CHECK_FALSE(reference::create(book_id::genesis, 51, 1, esv).has_value());
  }
  GIVEN("a verse outside the versification")
  {
    CHECK_FALSE(reference::create(book_id::genesis, 1, 32, esv).has_value());
  }
  GIVEN("the first and the last reference of the versification")
  {
    CHECK(reference::create(book_id::genesis, 1, 1, esv).has_value());
    CHECK(reference::create(book_id::revelation, 22, 21, esv).has_value());
    CHECK_FALSE(reference::create(book_id::revelation, 22, 22, esv).has_value());
  }
}

TEST_CASE("reference comparison", "[bible]")
{
  GIVEN("references ordered by book, then chapter, then verse")
  {
    static_assert(reference::create_unguarded(book_id::genesis, 1, 1) < reference::create_unguarded(book_id::genesis, 1, 2));
    static_assert(reference::create_unguarded(book_id::genesis, 1, 31) < reference::create_unguarded(book_id::genesis, 2, 1));
    static_assert(reference::create_unguarded(book_id::genesis, 50, 26) < reference::create_unguarded(book_id::exodus, 1, 1));
  }
  GIVEN("equal references")
  {
    static_assert(reference::create_unguarded(book_id::john, 3, 16) == reference::create_unguarded(book_id::john, 3, 16));
    static_assert(reference::create_unguarded(book_id::john, 3, 16) != reference::create_unguarded(book_id::john1, 3, 16));
  }
}

TEST_CASE("reference format", "[bible]")
{
  CHECK(std::format("{}", reference::create_unguarded(book_id::john, 3, 16)) == "john 3, 16");
  CHECK(std::format("{}", reference::create_unguarded(book_id::john, 3, 16).chapter()) == "3");
}

} // namespace bibstd::bible
