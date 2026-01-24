#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/reference.hpp"

namespace bibstd::bible
{

///
/// Struct containing relevant information defining a bible passage.
///
struct passage_info final
{
  // Constructor
  passage_info(bible::reference reference, bible::translation translation);

  // Operators
  auto operator==(const passage_info&) const -> bool = default;

  // Variables
  bible::reference reference;
  bible::translation translation;
};

} // namespace bibstd::bible
