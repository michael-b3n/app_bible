#include "bible/reference_range.hpp"

#include <algorithm>

namespace bibstd::bible
{

///
///
reference_range::reference_range(reference first_and_last)
  : from_(first_and_last)
  , to_(first_and_last)
{
}

///
///
reference_range::reference_range(reference first, reference second)
  : from_(std::min(first, second))
  , to_(std::max(first, second))
{
}

///
/// TODO: This algorithm is correct but very inefficient.
auto reference_range::size() const -> std::uint32_t
{
  constexpr auto max_verses = std::uint32_t{31102};
  auto temp = from_;
  auto counter = std::uint32_t{0};
  while(temp <= to_ && counter <= max_verses)
  {
    ++counter;
    ++temp;
  }
  return counter;
}

///
///
auto reference_range::begin() const -> reference
{
  return from_;
}

///
///
auto reference_range::end() const -> reference
{
  return to_;
}

} // namespace bibstd::bible
