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
