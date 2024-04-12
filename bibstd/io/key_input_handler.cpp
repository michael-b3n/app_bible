#include "io/key_input_handler.hpp"
#include "util/const_bimap.hpp"
#include "util/enum_helpers.hpp"

#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace bibstd::io
{

///
///
key_input_handler::key_input_handler()
{
}

///
///
auto key_input_handler::register_global_key(const key_id_type key_id, const key_modifier mod, std::function<void()>&& callback) -> void
{
  static constexpr auto hotkey_id = 4410;
  static constexpr auto key_modifier_map = util::const_bimap(
    std::pair{key_modifier::alt, MOD_ALT},
    std::pair{key_modifier::control, MOD_CONTROL},
    std::pair{key_modifier::shift, MOD_SHIFT},
    std::pair{key_modifier::alt_control, MOD_ALT | MOD_CONTROL},
    std::pair{key_modifier::alt_shift, MOD_ALT | MOD_SHIFT},
    std::pair{key_modifier::control_shift, MOD_CONTROL | MOD_SHIFT});
  // Register the hotkey
  if(RegisterHotKey(nullptr, hotkey_id, key_modifier_map.at(mod), key_id))
  {
    util::log_debug(std::format("[io] Registered hotkey: key_id={}, key_modifier={}", key_id, util::to_integral(mod)));
    MSG msg;
    while(GetMessage(&msg, nullptr, 0, 0))
    {
      if(msg.message == WM_HOTKEY && msg.wParam == hotkey_id)
      {
        callback();
      }
      DispatchMessage(&msg);
    }
  }
  else
  {
    // Handle registration failure
    util::log_error(std::format("[io] Failed to register hotkey: key_id={}, key_modifier={}", key_id, util::to_integral(mod)));
  }
}

} // namespace bibstd::io
