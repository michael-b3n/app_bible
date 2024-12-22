#include <math/is_equal.hpp>
#include <math/value_range.hpp>

#include <catch2/catch_test_macros.hpp>

namespace bibstd::math
{

TEST_CASE("value_range", "[math]")
{
  // Typedefs
  using value_range_type = value_range<double>;

  GIVEN("empty double value_range")
  {
    constexpr auto first_range = value_range_type(1.0, 1.0);
    constexpr auto second_range = value_range_type(2.0, 2.0);
    // size
    static_assert(math::is_equal(value_range_type::size(first_range), 0.0));
    static_assert(math::is_equal(value_range_type::size(second_range), 0.0));
    // contains
    static_assert(!value_range_type::contains(first_range, second_range));
    static_assert(!value_range_type::contains(second_range, first_range));
    // overlap
    static_assert(!value_range_type::overlaps(first_range, second_range));
    static_assert(!value_range_type::overlap(first_range, second_range));
    // adjacent
    static_assert(!value_range_type::adjacent(first_range, second_range));
  }

  GIVEN("overlapping double value_range")
  {
    constexpr auto first_range = value_range_type(0.0, 1.0);
    constexpr auto second_range = value_range_type(0.5, 2.0);
    // size
    static_assert(math::is_equal(value_range_type::size(first_range), 1.0));
    static_assert(math::is_equal(value_range_type::size(second_range), 1.5));
    // contains
    static_assert(!value_range_type::contains(first_range, second_range));
    static_assert(!value_range_type::contains(second_range, first_range));
    // overlap
    static_assert(value_range_type::overlaps(first_range, second_range));
    static_assert(value_range_type::overlap(first_range, second_range) == value_range_type(0.5, 1.0));
    // adjacent
    static_assert(!value_range_type::adjacent(first_range, second_range));
  }

  GIVEN("containing double value_range")
  {
    constexpr auto first_range = value_range_type(0.5, 1.5);
    constexpr auto second_range = value_range_type(0.5, 2.0);
    // size
    static_assert(math::is_equal(value_range_type::size(first_range), 1.0));
    static_assert(math::is_equal(value_range_type::size(second_range), 1.5));
    // contains
    static_assert(!value_range_type::contains(first_range, second_range));
    static_assert(value_range_type::contains(second_range, first_range));
    // overlap
    static_assert(value_range_type::overlaps(first_range, second_range));
    static_assert(value_range_type::overlap(first_range, second_range) == first_range);
    // adjacent
    static_assert(!value_range_type::adjacent(first_range, second_range));
  }

  GIVEN("adjacent double value_range")
  {
    constexpr auto first_range = value_range_type(0.0, 1.5);
    constexpr auto second_range = value_range_type(1.5, 2.0);
    // size
    static_assert(math::is_equal(value_range_type::size(first_range), 1.5));
    static_assert(math::is_equal(value_range_type::size(second_range), 0.5));
    // contains
    static_assert(!value_range_type::contains(first_range, second_range));
    static_assert(!value_range_type::contains(second_range, first_range));
    // overlap
    static_assert(value_range_type::overlaps(first_range, second_range));
    static_assert(value_range_type::overlap(first_range, second_range) == value_range_type(1.5, 1.5));
    // adjacent
    static_assert(value_range_type::adjacent(first_range, second_range));
  }
}

} // namespace bibstd::math
