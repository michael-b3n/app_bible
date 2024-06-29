#pragma once

#include "math/arithmetic.hpp"

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
  /// Checks if a `value_range` is empty.
  /// \param range Range object
  /// \return true, if `value_range` is emtpy
  ///
  static auto empty(const value_range& range) -> bool;

  ///
  /// Checks if `subrange` is fully contained in `range`.
  /// \param range Range that might contain `subrange`
  /// \param subrange Subrange that might be contained in `range`
  /// \return true, if `subrange` is fully contained in `range`
  ///
  static auto contains(const value_range& range, const value_range& subrange) -> bool;

  ///
  /// Checks if two ranges overlap.
  /// \param first First range
  /// \param second Second range
  /// \return true, if two ranges overlap, false otherwise
  ///
  static auto overlaps(const value_range& first, const value_range& second) -> bool;

  ///
  /// Get the range identifying an overlap between two ranges.
  /// \param first First range
  /// \param second Second range
  /// \return range describing overlap of `first` and `second`
  ///
  static auto overlap(const value_range& first, const value_range& second) -> value_range;

  ///
  /// Checks if two ranges are adjacent to another.
  /// \param first First range
  /// \param second Second range
  /// \return true if adjacent, false otherwise
  ///
  static auto adjacent(const value_range& first, const value_range& second) -> bool;

  // Constructor
  constexpr value_range(value_type begin, value_type size);

  // Operators
  constexpr auto operator==(const value_range&) const -> bool = default;

  // Variables
  value_type from;
  value_type to;
};

///
///
template<arithmetic_type ValueType>
constexpr value_range<ValueType>::value_range(value_type from_, value_type to_)
  : from{std::min(from_, to_)}
  , to{std::max(from_, to_)}
{
}

///
///
template<arithmetic_type ValueType>
auto value_range<ValueType>::empty(const value_range& range) -> bool
{
  return range.from == range.to;
}

///
///
template<arithmetic_type ValueType>
auto value_range<ValueType>::contains(const value_range& range, const value_range& subrange) -> bool
{
  return range.from <= subrange.from && range.to >= subrange.to;
}

///
///
template<arithmetic_type ValueType>
auto value_range<ValueType>::overlaps(const value_range& first, const value_range& second) -> bool
{
  return first.from <= second.to && second.from <= first.to;
}

///
///
template<arithmetic_type ValueType>
auto value_range<ValueType>::overlap(const value_range& first, const value_range& second) -> value_range
{
  const auto begin = std::max(second.begin, first.begin);
  const auto to = std::min(second.to, first.to);
  return value_range{begin, to};
}

///
///
template<arithmetic_type ValueType>
auto value_range<ValueType>::adjacent(const value_range& first, const value_range& second) -> bool
{
  return first.begin == second.to || second.begin == first.to;
}

} // namespace bibstd::math
