#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace bibstd::system
{

///
/// Cursor base class for all system implementations.
///
struct cursor_base
{
  // Typedefs
  ///
  /// Cursor coordinates in display coordinate system.
  ///
  struct coordinates final
  {
    int x;
    int y;
  };
};

} // namespace bibstd::system
