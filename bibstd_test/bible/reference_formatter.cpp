#include <bibstd/bible/reference_formatter.hpp>
#include <bibstd/util/exception.hpp>

#include <catch2/catch_test_macros.hpp>

namespace bibstd::bible
{
namespace
{

constexpr auto ref(const book_id book, const std::uint32_t chapter, const std::uint32_t verse) -> reference
{
  return reference::create_unguarded(book, chapter, verse);
}

auto format(const util::language language, const reference_range& range) -> std::string
{
  return reference_formatter{}(language, range);
}

} // namespace

TEST_CASE("reference_formatter formats a single verse", "[bible]")
{
  const auto range = reference_range{ref(book_id::john, 3, 16)};
  CHECK(format(util::language::german, range) == "Johannes 3, 16");
  CHECK(format(util::language::english, range) == "John 3, 16");
}

TEST_CASE("reference_formatter formats a verse range", "[bible]")
{
  const auto range = reference_range{ref(book_id::john, 3, 16), ref(book_id::john, 3, 18)};
  CHECK(format(util::language::german, range) == "Johannes 3, 16 - 18");
  CHECK(format(util::language::english, range) == "John 3, 16 - 18");
}

TEST_CASE("reference_formatter formats a chapter range", "[bible]")
{
  const auto range = reference_range{ref(book_id::john, 3, 16), ref(book_id::john, 4, 2)};
  CHECK(format(util::language::german, range) == "Johannes 3, 16 - 4, 2");
  CHECK(format(util::language::english, range) == "John 3, 16 - 4, 2");
}

TEST_CASE("reference_formatter formats a range over several books", "[bible]")
{
  const auto range = reference_range{ref(book_id::genesis, 50, 26), ref(book_id::exodus, 1, 1)};
  CHECK(format(util::language::german, range) == "1.Mose 50, 26 - 2.Mose 1, 1");
  CHECK(format(util::language::english, range) == "Genesis 50, 26 - Exodus 1, 1");
}

TEST_CASE("reference_formatter rejects an unsupported language", "[bible]")
{
  const auto range = reference_range{ref(book_id::john, 3, 16)};
  CHECK_THROWS_AS(format(static_cast<util::language>(-1), range), util::exception);
}

} // namespace bibstd::bible
