#include <bibstd/data/plane.hpp>
#include <bibstd/util/contains.hpp>

#include <catch2/catch_test_macros.hpp>

namespace bibstd::data
{

TEST_CASE("plane", "[data]")
{
  GIVEN("plane_const")
  {
    // clang-format off
    static constexpr auto plane = plane_const<int, 3, 3>({
     1, 2, 3,
     4, 5, 6,
     7, 8, 9});
    // clang-format on
    static constexpr auto view = plane_view<const int>(plane);
    static constexpr auto test_area = decltype(view)::area_type(math::coordinates{1, 1}, 1u, 2u);

    static_assert(view.size() == 9);
    static_assert(view.at(0) == 1);
    static_assert(view.at(1) == 2);
    static_assert(view.at(8) == 9);
    static_assert(view.width() == 3);
    static_assert(view.height() == 3);
    static_assert(view.data_view_size(test_area) == 2);
    static_assert(util::contains(view.data_view(test_area), 5));
    static_assert(util::contains(view.data_view(test_area), 8));
  }
}

} // namespace bibstd::data
