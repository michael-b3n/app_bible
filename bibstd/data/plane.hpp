#pragma once

#include <cstdint>
#include <vector>

namespace bibstd::data
{

///
/// Represents a plane of objects of type T.
/// The plane has a width and a height.
///
template<typename T>
struct plane final
{
  // Typedefs
  using value_type = T;
  using data_type = std::vector<value_type>;

  // Structors
  plane(std::uint32_t width, std::uint32_t height);
  plane() = default;

  // Variables
  std::uint32_t width{0};
  std::uint32_t height{0};
  data_type data;
};

///
///
template<typename T>
plane<T>::plane(std::uint32_t width_, std::uint32_t height_)
  : width{width_}
  , height{height_}
  , data(static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_))
{
}

} // namespace bibstd::data
