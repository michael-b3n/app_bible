#pragma once

#include "math/arithmetic.hpp"
#include "math/coordinates.hpp"
#include "math/value_range.hpp"

#include <cassert>

namespace bibstd::math
{

///
/// Rectangle in a 2D plane on basis of an arithmetic value type.
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
  static constexpr auto overlap(const rect<value_type>& first, const rect<value_type>& second) -> rect<value_type>;

  ///
  /// Check if subrectangle is contained by another rectangle.
  /// \param rectangle Rectangle
  /// \param subrectangle Subrectangle
  /// \return true if subrectangle is contained in rectangle, false otherwise
  ///
  static constexpr auto contains(const rect<value_type>& rectangle, const rect<value_type>& subrectangle) -> bool;

public: // Constructors
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
  constexpr auto left_lower_coordinates() const -> coordinates_type;

  ///
  /// Get the lower right coordinates of the rectangle.
  /// \return coordinates at lower right position
  ///
  constexpr auto right_lower_coordinates() const -> coordinates_type;

  ///
  /// Get the upper left coordinates of the rectangle.
  /// \return coordinates at upper left position
  ///
  constexpr auto left_upper_coordinates() const -> coordinates_type;

  ///
  /// Get the upper right coordinates of the rectangle.
  /// \return coordinates at upper right position
  ///
  constexpr auto right_upper_coordinates() const -> coordinates_type;

private: // Variables
  coordinates_type left_lower_coord_;
  coordinates_type right_upper_coord_;
};

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::overlap(const rect<value_type>& first, const rect<value_type>& second) -> rect<value_type>
{
  const auto first_x_range = value_range<value_type>(first.left_lower_coordinates().axis_value(0), first.right_lower_coordinates().axis_value(0));
  const auto first_y_range = value_range<value_type>(first.left_lower_coordinates().axis_value(1), first.left_upper_coordinates().axis_value(1));
  const auto second_x_range = value_range<value_type>(second.left_lower_coordinates().axis_value(0), second.right_lower_coordinates().axis_value(0));
  const auto second_y_range = value_range<value_type>(second.left_lower_coordinates().axis_value(1), second.left_upper_coordinates().axis_value(1));
  const auto x_overlap = value_range<value_type>::overlap(first_x_range, second_x_range);
  const auto y_overlap = value_range<value_type>::overlap(first_y_range, second_y_range);
  return rect<value_type>(coordinates_type(x_overlap.from, y_overlap.from), coordinates_type(x_overlap.to, y_overlap.to));
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::contains(const rect<value_type>& rectangle, const rect<value_type>& subrectangle) -> bool
{
  const auto left_lower_contained = rectangle.left_lower_coordinates().axis_value(0) <= subrectangle.left_lower_coordinates().axis_value(0) &&
                                    rectangle.left_lower_coordinates().axis_value(1) <= subrectangle.left_lower_coordinates().axis_value(1);
  const auto right_upper_contained = rectangle.right_upper_coordinates().axis_value(0) >= subrectangle.right_upper_coordinates().axis_value(0) &&
                                     rectangle.right_upper_coordinates().axis_value(1) >= subrectangle.right_upper_coordinates().axis_value(1);
  return left_lower_contained && right_upper_contained;
}

///
///
template<arithmetic_type ValueType>
constexpr rect<ValueType>::rect(coordinates_type first, coordinates_type second)
  : left_lower_coord_{std::min(first.axis_value(0), second.axis_value(0)), std::min(first.axis_value(1), second.axis_value(1))}
  , right_upper_coord_{std::max(first.axis_value(0), second.axis_value(0)), std::max(first.axis_value(1), second.axis_value(1))}
{
  const auto x1 = left_lower_coord_.axis_value(0);
  const auto x2 = right_upper_coord_.axis_value(0);
  const auto y1 = left_lower_coord_.axis_value(1);
  const auto y2 = right_upper_coord_.axis_value(1);
  if(!arithmetic::multiply(arithmetic::subtract(x2, x1), arithmetic::subtract(y2, y1)).has_value())
  {
    THROW_EXCEPTION(util::exception("Invalid rect size: overflow"));
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::horizontal_range() const -> value_type
{
  const auto x1 = left_lower_coord_.axis_value(0);
  const auto x2 = right_upper_coord_.axis_value(0);
  assert(x2 >= x1);
  return x2 - x1;
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::vertical_range() const -> value_type
{
  const auto y1 = left_lower_coord_.axis_value(1);
  const auto y2 = right_upper_coord_.axis_value(1);
  assert(y2 >= y1);
  return y2 - y1;
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
constexpr auto rect<ValueType>::left_lower_coordinates() const -> coordinates_type
{
  return left_lower_coord_;
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::right_lower_coordinates() const -> coordinates_type
{
  return coordinates_type{right_upper_coord_.axis_value(0), left_lower_coord_.axis_value(1)};
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::left_upper_coordinates() const -> coordinates_type
{
  return coordinates_type{left_lower_coord_.axis_value(0), right_upper_coord_.axis_value(1)};
}

///
///
template<arithmetic_type ValueType>
constexpr auto rect<ValueType>::right_upper_coordinates() const -> coordinates_type
{
  return right_upper_coord_;
}

} // namespace bibstd::math
