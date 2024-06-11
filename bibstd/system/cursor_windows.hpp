#pragma once

#include "system/cursor_base.hpp"
#include "util/exception.hpp"
#include "util/log.hpp"
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace bibstd::system
{

///
/// Cursor class for windows implementation.
///
class cursor final : public cursor_base
{
public: // Static accessors
  ///
  /// Get the virtual screen metrics.
  /// \return screen metrics
  ///
  static inline auto position() -> coordinates;
};

///
///
inline auto cursor::position() -> coordinates
{
  POINT point;
  if(!GetCursorPos(&point))
  {
    THROW_EXCEPTION(util::exception("Failed to get cursor position"));
  }
  return coordinates{.x = point.x, .y = point.y};
}

} // namespace bibstd::system
