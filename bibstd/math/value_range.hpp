#pragma once

#include "math/arithmetic.hpp"
#include "math/is_equal.hpp"
#include "util/enum.hpp"

#include <optional>

namespace bibstd::math
{

///
/// Value range type.
/// Range definition for integer types: `[begin, end)`
/// Range definition for floating point: `[begin, end]`
///
template<arithmetic_type ValueType>
struct value_range final
{
  // Typedefs
  using value_type = ValueType;

  // Static functions
  ///
  /// Check if the `value_range` is empty (`begin == end`).
  /// \param range Range object
  /// \return true if `value_range` is empty, false otherwise
  ///
  static constexpr auto empty(const value_range& range) -> bool;

  ///
  /// Get the size (defined as `size = end - begin` where `end > begin`) of a `value_range`.
  /// \param range Range object
  /// \return size of `value_range` (type std::size_t for integer types, type double for floating point types)
  ///
  static constexpr auto size(const value_range& range) -> auto;

  ///
  /// Checks if `subrange` is fully contained in `range`.
  /// \param range Range that might contain `subrange`
  /// \param subrange Subrange that might be contained in `range`
  /// \return true, if `subrange` is fully contained in `range`
  ///
  static constexpr auto contains(const value_range& range, const value_range& subrange) -> bool;

  ///
  /// Checks if `value` is fully contained in `range`.
  /// \param range Range that might contain `subrange`
  /// \param value Value that might be contained in `range`
  /// \return true, if `value` is fully contained in `range`
  ///
  static constexpr auto contains(const value_range& range, value_type value) -> bool;

  ///
  /// Checks if two ranges overlap.
  /// Empty integer ranges will never overlap with another range.
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
  /// Checks if two ranges are adjacent end another.
  /// Empty integer ranges will never be adjacent to another range.
  /// \param first First range
  /// \param second Second range
  /// \return true if adjacent, false otherwise
  ///
  static constexpr auto adjacent(const value_range& first, const value_range& second) -> bool;

  // Constructor
  ///
  /// Value range constructor taking begin and end of a value range.
  /// Range definition for integer types: `[begin, end)`
  /// Range definition for floating point: `[begin, end]`
  ///
  constexpr value_range(value_type begin, value_type end);

  // Operators
  constexpr auto operator==(const value_range& other) const -> bool;

  // Variables
  value_type begin;
  value_type end;
};

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::empty(const value_range& range) -> bool
{
  return range.begin == range.end;
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::size(const value_range& range) -> auto
{
  if constexpr(std::integral<value_type>)
  {
    return static_cast<std::size_t>(range.end - range.begin);
  }
  else
  {
    return static_cast<double>(range.end - range.begin);
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::contains(const value_range& range, const value_range& subrange) -> bool
{
  if constexpr(std::integral<value_type>)
  {
    const auto empty_found = empty(range) || empty(subrange);
    return !empty_found && range.begin <= subrange.begin && range.end >= subrange.end;
  }
  else
  {
    return range.begin <= subrange.begin && range.end >= subrange.end;
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::contains(const value_range& range, const value_type value) -> bool
{
  if constexpr(std::integral<value_type>)
  {
    return !empty(range) && range.begin <= value && range.end > value;
  }
  else
  {
    return range.begin <= value && range.end >= value;
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::overlaps(const value_range& first, const value_range& second) -> bool
{
  if constexpr(std::integral<value_type>)
  {
    const auto empty_found = empty(first) || empty(second);
    return !empty_found && first.end > second.begin && second.end > first.begin;
  }
  else
  {
    return first.begin <= second.end && second.begin <= first.end;
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::overlap(const value_range& first, const value_range& second)
  -> std::optional<value_range>
{
  if(overlaps(first, second))
  {
    const auto begin = std::max(second.begin, first.begin);
    const auto end = std::min(second.end, first.end);
    return value_range{begin, end};
  }
  return std::nullopt;
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::adjacent(const value_range& first, const value_range& second) -> bool
{
  if constexpr(std::integral<value_type>)
  {
    const auto empty_found = empty(first) || empty(second);
    return !empty_found && (is_equal(first.begin, second.end) || is_equal(second.begin, first.end));
  }
  else
  {
    return is_equal(first.begin, second.end) || is_equal(second.begin, first.end);
  }
}

///
///
template<arithmetic_type ValueType>
constexpr value_range<ValueType>::value_range(value_type begin_, value_type to_)
  : begin{std::min(begin_, to_)}
  , end{std::max(begin_, to_)}
{
  if(!arithmetic::subtract(end, begin).has_value() || !arithmetic::add(begin, static_cast<value_type>(1)).has_value())
  {
    THROW_EXCEPTION(std::invalid_argument("invalid value_range arguments"));
  }
}

///
///
template<arithmetic_type ValueType>
constexpr auto value_range<ValueType>::operator==(const value_range& other) const -> bool
{
  return is_equal(begin, other.begin) && is_equal(end, other.end);
}

} // namespace bibstd::math

///
///
template<bibstd::math::arithmetic_type ValueType>
struct std::formatter<bibstd::math::value_range<ValueType>> : std::formatter<std::string>
{
  auto format(const bibstd::math::value_range<ValueType>& e, std::format_context& ctx) const
  {
    if constexpr(std::integral<ValueType>)
    {
      return formatter<std::string>::format(std::format("[{}, {})", e.begin, e.end), ctx);
    }
    else
    {
      return formatter<std::string>::format(std::format("[{}, {}]", e.begin, e.end), ctx);
    }
  }
};
