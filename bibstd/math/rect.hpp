#pragma once

#include "math/arithmetic.hpp"
#include "math/coordinates.hpp"
#include "math/value_range.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/enum.hpp"

#include <cassert>
#include <optional>

namespace bibstd::math
{

///
/// Rectangle in a 2D plane on basis of an arithmetic value type.
/// Horizontal and vertical range definition for integer types: `[begin, end)`
/// Horizontal and vertical range definition for floating point: `[begin, end]`
///
template<arithmetic_type ValueType>
class rect final
{
public: // Typedefs
  using value_type = ValueType;
  using coordinates_type = coordinates<value_type, 2>;

public: // Static functions
  ///
  /// Get the overlap of two rectangles.
  /// \param first First rectangle
  /// \param second Second rectangle
  /// \return the overlap of two rectangles as a rectangle
  ///
  static constexpr auto overlap(const rect<value_type>& first, const rect<value_type>& second)
    -> std::optional<rect<value_type>>;

  ///
  /// Check if subrectangle is contained by another rectangle.
  /// \param rectangle Rectangle
  /// \param subrectangle Subrectangle
  /// \return true if subrectangle is contained in rectangle, false otherwise
  ///
  static constexpr auto contains(const rect<value_type>& rectangle, const rect<value_type>& subrectangle) -> bool;

  ///
  /// Check if coordinates are contained by another rectangle.
  /// \param rectangle Rectangle
  /// \param coordinates Coordinates that shall be checked
  /// \return true if coordinates are contained in rectangle, false otherwise
  ///
  static constexpr auto contains(const rect<value_type>& rectangle, const coordinates_type& coordinates) -> bool;

public: // Constructors
  constexpr rect(coordinates_type origin, value_type width, value_type height);
  constexpr rect(coordinates_type first, coordinates_type second);

public: // Operators
  constexpr auto operator==(const rect&) const -> bool = default;

public: // Accessors
  ///
  /// Get the span from left to right of the rectangle.
  /// \return the horizontal span of the rect
  ///
  constexpr auto horizontal_range() const -> value_type;

  ///
  /// Get the span from lower to upper of the rectangle.
  /// \return the vertical span of the rect
  ///
  constexpr auto vertical_range() const -> value_type;

  ///
  /// Get the area of the rectangle.
  /// \return the area of the rect
  ///
  constexpr auto area() const -> value_type;

  ///
  /// Get the lower left coordinates of the rectangle.
  /// \return coordinates at lower left position
  ///
  constexpr auto origin() const -> coordinates_type;

  ///
  /// Get the center coordinates of the rectangle.
  /// If there is no exact center, the closest lower left coordinates are returned.
  /// \return center coordinates
  ///
  constexpr auto center() const -> coordinates_type;

private: // Variables
  value_range<value_type> horizontal_range_;
  value_range<value_type> vertical_range_;
};

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::overlap(const rect<value_type>& first, const rect<value_type>& second)
  -> std::optional<rect<value_type>>
{
  const auto x_overlap = value_range<value_type>::overlap(first.horizontal_range_, second.horizontal_range_);
  const auto y_overlap = value_range<value_type>::overlap(first.vertical_range_, second.vertical_range_);
  if(x_overlap && y_overlap)
  {
    return rect<value_type>(
      coordinates_type(x_overlap->begin, y_overlap->begin),
      value_range<value_type>::size(*x_overlap),
      value_range<value_type>::size(*y_overlap)
    );
  }
  return std::nullopt;
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::contains(const rect<value_type>& rectangle, const rect<value_type>& subrectangle) -> bool
{
  const auto x_contains = value_range<value_type>::contains(rectangle.horizontal_range_, subrectangle.horizontal_range_);
  const auto y_contains = value_range<value_type>::contains(rectangle.vertical_range_, subrectangle.vertical_range_);
  return x_contains && y_contains;
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::contains(const rect<value_type>& rectangle, const coordinates_type& coordinates) -> bool
{
  const auto x_contains = value_range<value_type>::contains(rectangle.horizontal_range_, coordinates.x());
  const auto y_contains = value_range<value_type>::contains(rectangle.vertical_range_, coordinates.y());
  return x_contains && y_contains;
}

///
///
template<arithmetic_type ValueType>
constexpr rect<ValueType>::rect(const coordinates_type origin, const value_type width, const value_type height)
  : horizontal_range_{origin.x(), arithmetic::add(origin.x(), width).value_or(0)}
  , vertical_range_{origin.y(), arithmetic::add(origin.y(), height).value_or(0)}
{
  const auto area =
    arithmetic::multiply(value_range<value_type>::size(horizontal_range_), value_range<value_type>::size(vertical_range_));
  if(!area.has_value() || !math::is_equal(area.value(), math::arithmetic::multiply(width, height)))
  {
    THROW_EXCEPTION(util::exception(std::format("invalid rect size: {}", util::to_string_view(area.error()))));
  }
}

///
///
template<arithmetic_type ValueType>
constexpr rect<ValueType>::rect(const coordinates_type first, const coordinates_type second)
  : horizontal_range_{ // clang-format off
      [&]
      {
        const auto min_x = std::min(first.x(), second.x());
        const auto max_x = arithmetic::add(std::max(first.x(), second.x()), value_type{1});
        if(!max_x.has_value())
        {
          THROW_EXCEPTION(util::exception(std::format("invalid max width: {}", util::to_string_view(max_x.error()))));
        }
        return value_range<value_type>(min_x, max_x.value());
      }()}
  , vertical_range_{
      [&]
      {
        const auto min_y = std::min(first.y(), second.y());
        const auto max_y = arithmetic::add(std::max(first.y(), second.y()), value_type{1});
        if(!max_y.has_value())
        {
          THROW_EXCEPTION(util::exception(std::format("invalid max height: {}", util::to_string_view(max_y.error()))));
        }
        return value_range<value_type>(min_y, max_y.value());
      }()
    } // clang-format on
{
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::horizontal_range() const -> value_type
{
  return value_range<value_type>::size(horizontal_range_);
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::vertical_range() const -> value_type
{
  return value_range<value_type>::size(vertical_range_);
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::area() const -> value_type
{
  return vertical_range() * horizontal_range();
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::origin() const -> coordinates_type
{
  return coordinates_type{horizontal_range_.begin, vertical_range_.begin};
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::center() const -> coordinates_type
{
  const auto horizontal_size = value_range<value_type>::size(horizontal_range_);
  const auto horizontal_offset = boost::numeric_cast<value_type>(horizontal_size / decltype(horizontal_size){2});
  const auto vertical_size = value_range<value_type>::size(vertical_range_);
  const auto vertical_offset = boost::numeric_cast<value_type>(vertical_size / decltype(vertical_size){2});
  return coordinates_type{horizontal_range_.begin + horizontal_offset, vertical_range_.begin + vertical_offset};
}

} // namespace bibstd::math
