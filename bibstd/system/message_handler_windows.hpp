#pragma once

#include <windows.h>

namespace bibstd::system
{

///
/// Filesystem base class for all system implementations.
///
struct message_handler
{
  ///
  /// Get and dispatch system message.
  ///
  static inline auto get_message() -> void;
};

///
///
inline auto message_handler::get_message() -> void
{
  static MSG msg;
  if(GetMessage(&msg, nullptr, 0, 0))
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
}

} // namespace bibstd::system
