#include <bibstd/bible/versification.hpp>

#include <catch2/catch_test_macros.hpp>

namespace bibstd::bible
{

TEST_CASE("versification name", "[bible]")
{
  constexpr auto v = versification_esv;
  static_assert(!v.name().empty());
}

TEST_CASE("versification count", "[bible]")
{
  constexpr auto v = versification_esv;
  static_assert(v.count() == 31103);
}

TEST_CASE("versification chapter_count", "[bible]")
{
  constexpr auto v = versification_esv;
  static_assert(v.chapter_count(book_id::genesis) == 50);
  static_assert(v.chapter_count(book_id::psalms) == 150);
  static_assert(v.chapter_count(book_id::john) == 21);
  static_assert(v.chapter_count(book_id::revelation) == 22);
}

TEST_CASE("versification verse_count", "[bible]")
{
  constexpr auto v = versification_esv;
  using ct = reference::chapter_type;
  static_assert(v.verse_count(book_id::genesis, ct{1}) == 31);
  static_assert(v.verse_count(book_id::genesis, ct{50}) == 26);
  static_assert(v.verse_count(book_id::psalms, ct{119}) == 176);
  static_assert(v.verse_count(book_id::psalms, ct{150}) == 6);
  static_assert(v.verse_count(book_id::john, ct{3}) == 36);
  static_assert(v.verse_count(book_id::revelation, ct{22}) == 21);
  static_assert(v.verse_count(book_id::genesis, ct{0}) == 0);
  static_assert(v.verse_count(book_id::genesis, ct{51}) == 0);
}

TEST_CASE("versification contains", "[bible]")
{
  constexpr auto v = versification_esv;
  static_assert(v.contains(reference::create_unguarded(book_id::genesis, 1, 1)));
  static_assert(v.contains(reference::create_unguarded(book_id::genesis, 1, 31)));
  static_assert(!v.contains(reference::create_unguarded(book_id::genesis, 1, 32)));
  static_assert(!v.contains(reference::create_unguarded(book_id::genesis, 51, 1)));
  static_assert(v.contains(reference::create_unguarded(book_id::revelation, 22, 21)));
  static_assert(!v.contains(reference::create_unguarded(book_id::revelation, 22, 22)));
}

TEST_CASE("versification validate", "[bible]")
{
  // clang-format off
  constexpr auto v = versification_esv;
  static_assert(v.validate(reference::create_unguarded(book_id::genesis, 1, 1)) == reference::create_unguarded(book_id::genesis, 1, 1));
  static_assert(v.validate(reference::create_unguarded(book_id::genesis, 51, 1)) == reference::create_unguarded(book_id::genesis, 50, 1));
  static_assert(v.validate(reference::create_unguarded(book_id::genesis, 1, 32)) == reference::create_unguarded(book_id::genesis, 1, 31));
  static_assert(v.validate(reference::create_unguarded(book_id::revelation, 22, 21)) == reference::create_unguarded(book_id::revelation, 22, 21));
  // clang-format on
}

TEST_CASE("versification next", "[bible]")
{
  // clang-format off
  constexpr auto v = versification_esv;
  static_assert(v.next(reference::create_unguarded(book_id::genesis, 1, 1)) == reference::create_unguarded(book_id::genesis, 1, 2));
  static_assert(v.next(reference::create_unguarded(book_id::genesis, 1, 31)) == reference::create_unguarded(book_id::genesis, 2, 1));
  static_assert(v.next(reference::create_unguarded(book_id::genesis, 50, 26)) == reference::create_unguarded(book_id::exodus, 1, 1));
  static_assert(!v.next(reference::create_unguarded(book_id::revelation, 22, 21)).has_value());
  static_assert(!v.next(reference::create_unguarded(book_id::genesis, 51, 1)).has_value());
  // clang-format on
}

TEST_CASE("versification prev", "[bible]")
{
  // clang-format off
  constexpr auto v = versification_esv;
  static_assert(v.prev(reference::create_unguarded(book_id::genesis, 1, 2)) == reference::create_unguarded(book_id::genesis, 1, 1));
  static_assert(v.prev(reference::create_unguarded(book_id::genesis, 2, 1)) == reference::create_unguarded(book_id::genesis, 1, 31));
  static_assert(v.prev(reference::create_unguarded(book_id::exodus, 1, 1)) == reference::create_unguarded(book_id::genesis, 50, 26));
  static_assert(!v.prev(reference::create_unguarded(book_id::genesis, 1, 1)).has_value());
  static_assert(!v.prev(reference::create_unguarded(book_id::genesis, 51, 1)).has_value());
  // clang-format on
}

TEST_CASE("versification operator==", "[bible]")
{
  constexpr auto v1 = versification_esv;
  constexpr auto v2 = versification_esv;
  static_assert(v1 == v2);
}

TEST_CASE("versification size", "[bible]")
{
  // clang-format off
  constexpr auto v = versification_esv;
  static_assert(v.size(reference_range{reference::create_unguarded(book_id::genesis, 1, 1), reference::create_unguarded(book_id::genesis, 1, 31)}) == 31);
  static_assert(v.size(reference_range{reference::create_unguarded(book_id::john, 3, 1), reference::create_unguarded(book_id::john, 3, 16)}) == 16);
  static_assert(v.size(reference_range{reference::create_unguarded(book_id::genesis, 1, 1), reference::create_unguarded(book_id::genesis, 2, 1)}) == 32);
  // clang-format on
}

} // namespace bibstd::bible
