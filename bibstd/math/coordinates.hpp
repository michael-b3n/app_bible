#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

namespace bibstd::math
{

///
/// Coordinates for definition of a point in n-dim space.
///
template<typename ValueType, std::size_t D>
class coordinates final
{
public: // Typedefs
  using value_type = ValueType;

public: // Constructors
  template<typename... T>
  constexpr coordinates(T... coords);

public: // Accessors
  ///
  /// Get the coordinates value of the defined dimension number.
  /// \param dim Dimension number
  /// \return coordinates value of dimension `dim`
  ///
  [[nodiscard]] constexpr auto axis_value(std::size_t dim) const -> value_type;

public: // Operators
  constexpr auto operator==(const coordinates<value_type, D>&) const -> bool = default;

private: // Variables
  std::array<value_type, D> coordinates_;
};

///
///
template<typename... T>
coordinates(T...) -> coordinates<std::common_type_t<T...>, sizeof...(T)>;

///
///
template<typename ValueType, std::size_t D>
template<typename... T>
constexpr coordinates<ValueType, D>::coordinates(T... coords)
  : coordinates_{std::array{static_cast<value_type>(coords)...}}
{
}

///
///
template<typename ValueType, std::size_t D>
constexpr auto coordinates<ValueType, D>::axis_value(const std::size_t dim) const -> value_type
{
  return coordinates_.at(dim);
}

} // namespace bibstd::math
