#pragma once

#include "math/arithmetic.hpp"
#include "math/is_equal.hpp"
#include "util/enum.hpp"

#include <optional>

namespace bibstd::math
{

///
/// Value range type.
///
template<arithmetic_type ValueType>
struct value_range final
{
  // Typedefs
  using value_type = ValueType;

  // Static functions
  ///
  /// Get the size (defined as `size = to - from` where `to > from`) of a `value_range`.
  /// \param range Range object
  /// \return size of `value_range`
  ///
  static constexpr auto size(const value_range& range) -> value_type;

  ///
  /// Checks if `subrange` is fully contained in `range`.
  /// \param range Range that might contain `subrange`
  /// \param subrange Subrange that might be contained in `range`
  /// \return true, if `subrange` is fully contained in `range`
  ///
  static constexpr auto contains(const value_range& range, const value_range& subrange) -> bool;

  ///
  /// Checks if two ranges overlap.
  /// \param first First range
  /// \param second Second range
  /// \return true, if two ranges overlap, false otherwise
  ///
  static constexpr auto overlaps(const value_range& first, const value_range& second) -> bool;

  ///
  /// Get the range identifying an overlap between two ranges.
  /// \param first First range
  /// \param second Second range
  /// \return optional range describing overlap of `first` and `second`, std::nullopt if no overlap is present
  ///
  static constexpr auto overlap(const value_range& first, const value_range& second) -> std::optional<value_range>;

  ///
  /// Checks if two ranges are adjacent to another.
  /// \param first First range
  /// \param second Second range
  /// \return true if adjacent, false otherwise
  ///
  static constexpr auto adjacent(const value_range& first, const value_range& second) -> bool;

  // Constructor
  constexpr value_range(value_type begin, value_type size);

  // Operators
  constexpr auto operator==(const value_range& other) const -> bool;

  // Variables
  value_type from;
  value_type to;
};

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::size(const value_range& range) -> value_type
{
  return range.to - range.from;
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::contains(const value_range& range, const value_range& subrange) -> bool
{
  return range.from <= subrange.from && range.to >= subrange.to;
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::overlaps(const value_range& first, const value_range& second) -> bool
{
  return first.from <= second.to && second.from <= first.to;
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::overlap(const value_range& first, const value_range& second)
  -> std::optional<value_range>
{
  if(overlaps(first, second))
  {
    const auto begin = std::max(second.from, first.from);
    const auto to = std::min(second.to, first.to);
    return value_range{begin, to};
  }
  return std::nullopt;
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::adjacent(const value_range& first, const value_range& second) -> bool
{
  return is_equal(first.from, second.to) || is_equal(second.from, first.to);
}

///
///
template<arithmetic_type ValueType>
constexpr value_range<ValueType>::value_range(value_type from_, value_type to_)
  : from{std::min(from_, to_)}
  , to{std::max(from_, to_)}
{
  const auto size = arithmetic::subtract(to, from);
  if(!size.has_value())
  {
    THROW_EXCEPTION(util::exception(std::format("Invalid value_range size: {}", util::to_string_view(size.error()))));
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::operator==(const value_range& other) const -> bool
{
  return is_equal(from, other.from) && is_equal(to, other.to);
}

} // namespace bibstd::math
