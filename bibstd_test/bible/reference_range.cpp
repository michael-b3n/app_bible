#include <bibstd/bible/reference_range.hpp>

#include <catch2/catch_test_macros.hpp>

#include <format>

namespace bibstd::bible
{
namespace
{

constexpr auto ref(const book_id book, const std::uint32_t chapter, const std::uint32_t verse) -> reference
{
  return reference::create_unguarded(book, chapter, verse);
}

} // namespace

TEST_CASE("reference_range single reference", "[bible]")
{
  constexpr auto range = reference_range{ref(book_id::john, 3, 16)};
  static_assert(range.begin() == ref(book_id::john, 3, 16));
  static_assert(range.end() == range.begin());
}

TEST_CASE("reference_range orders its bounds", "[bible]")
{
  GIVEN("bounds in order")
  {
    constexpr auto range = reference_range{ref(book_id::john, 3, 16), ref(book_id::john, 3, 18)};
    static_assert(range.begin() == ref(book_id::john, 3, 16));
    static_assert(range.end() == ref(book_id::john, 3, 18));
  }
  GIVEN("bounds in reverse order")
  {
    // The constructor sorts its bounds, so begin is always the first reference of the range.
    constexpr auto range = reference_range{ref(book_id::john, 3, 18), ref(book_id::john, 3, 16)};
    static_assert(range.begin() == ref(book_id::john, 3, 16));
    static_assert(range.end() == ref(book_id::john, 3, 18));
  }
  GIVEN("bounds in different books")
  {
    constexpr auto range = reference_range{ref(book_id::exodus, 1, 1), ref(book_id::genesis, 50, 26)};
    static_assert(range.begin() == ref(book_id::genesis, 50, 26));
    static_assert(range.end() == ref(book_id::exodus, 1, 1));
  }
}

TEST_CASE("reference_range comparison", "[bible]")
{
  static_assert(reference_range{ref(book_id::john, 3, 16)} == reference_range{ref(book_id::john, 3, 16)});
  static_assert(
    reference_range{ref(book_id::john, 3, 16), ref(book_id::john, 3, 18)} ==
    reference_range{ref(book_id::john, 3, 18), ref(book_id::john, 3, 16)}
  );
  static_assert(reference_range{ref(book_id::john, 3, 16)} != reference_range{ref(book_id::john, 3, 18)});
}

TEST_CASE("reference_range format", "[bible]")
{
  CHECK(std::format("{}", reference_range{ref(book_id::john, 3, 16), ref(book_id::john, 3, 18)}) == "john 3, 16 - john 3, 18");
}

} // namespace bibstd::bible
