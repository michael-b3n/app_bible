#pragma once

#include "app_framework/active_worker.hpp"
#include "app_framework/task_queue.hpp"
#include "system/hotkey_base.hpp"
#include "util/const_bimap.hpp"
#include "util/enum_helpers.hpp"
#include "util/log.hpp"
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <functional>
#include <mutex>
#include <ranges>

namespace bibstd::system
{

///
/// Key input class for windows implementation.
///
class hotkey final : public hotkey_base
{
public: // Typedefs
#include BOOST_PP_UPDATE_COUNTER()
  using active_worker_type = app_framework::active_worker<BOOST_PP_COUNTER>;

public: // Static modifiers
  static inline auto init() -> util::scoped_guard;
  static inline auto register_callback(key key, key_modifier mod, std::function<void()>&& callback) -> void;
  static inline auto unregister_callback(key key, key_modifier mod) -> void;

private: // Constants
  static constexpr auto key_map = util::const_bimap(
    std::pair{key::vk_a, 0x41},
    std::pair{key::vk_b, 0x42},
    std::pair{key::vk_c, 0x43},
    std::pair{key::vk_d, 0x44},
    std::pair{key::vk_e, 0x45},
    std::pair{key::vk_f, 0x46},
    std::pair{key::vk_g, 0x47},
    std::pair{key::vk_h, 0x48},
    std::pair{key::vk_i, 0x49},
    std::pair{key::vk_j, 0x4A},
    std::pair{key::vk_k, 0x4B},
    std::pair{key::vk_l, 0x4C},
    std::pair{key::vk_m, 0x4D},
    std::pair{key::vk_n, 0x4E},
    std::pair{key::vk_o, 0x4F},
    std::pair{key::vk_p, 0x50},
    std::pair{key::vk_q, 0x51},
    std::pair{key::vk_r, 0x52},
    std::pair{key::vk_s, 0x53},
    std::pair{key::vk_t, 0x54},
    std::pair{key::vk_u, 0x55},
    std::pair{key::vk_v, 0x56},
    std::pair{key::vk_w, 0x57},
    std::pair{key::vk_x, 0x58},
    std::pair{key::vk_y, 0x59},
    std::pair{key::vk_z, 0x5A});

  static constexpr auto key_modifier_map = util::const_bimap(
    std::pair{key_modifier::alt, MOD_ALT},
    std::pair{key_modifier::control, MOD_CONTROL},
    std::pair{key_modifier::shift, MOD_SHIFT},
    std::pair{key_modifier::alt_control, MOD_ALT | MOD_CONTROL},
    std::pair{key_modifier::alt_shift, MOD_ALT | MOD_SHIFT},
    std::pair{key_modifier::control_shift, MOD_CONTROL | MOD_SHIFT});

private: // Static helpers
  static inline auto next_hotkey_id() -> int;
  static inline auto get_message() -> void;
  static inline auto message_handler(const MSG& msg) -> void;

private:
  inline static std::atomic_int hotkey_id_{0};
  inline static std::atomic_bool listen_to_msg_{true};
  inline static std::atomic<std::optional<DWORD>> windows_thread_id_{std::nullopt};
  inline static app_framework::task_queue register_queue{};
  inline static std::map<int, std::function<void()>> callback_map_{};
  inline static std::map<std::pair<key, key_modifier>, int> id_map_{};
};

///
///
inline auto hotkey::init() -> util::scoped_guard
{
  decltype(auto) worker_guard = active_worker_type::start(
    []()
    {
      windows_thread_id_ = GetCurrentThreadId();
      active_worker_type::queue_task(get_message);
    });
  return util::scoped_guard(
    [guard = std::move(worker_guard)]()
    {
      listen_to_msg_ = false;
      active_worker_type::run_task(
        []() { std::ranges::for_each(callback_map_ | std::views::keys, [](const auto id) { UnregisterHotKey(nullptr, id); }); });
      PostThreadMessage(windows_thread_id_.load().value(), WM_QUIT, 0, 0);
    });
}

///
///
inline auto hotkey::register_callback(const key key, const key_modifier mod, std::function<void()>&& callback) -> void
{
  assert(windows_thread_id_.load().has_value());
  const auto hotkey_id = next_hotkey_id();
  register_queue.queue(
    [key, mod, hotkey_id, callback]()
    {
      callback_map_[hotkey_id] = callback;
      id_map_[std::pair{key, mod}] = hotkey_id;
      if(!RegisterHotKey(nullptr, hotkey_id, key_modifier_map.at(mod), key_map.at(key)))
      {
        LOG_ERROR(log_channel, "Failed to register hotkey: key={}, modifier={}", util::to_integral(mod), util::to_integral(key));
      }
    });
  PostThreadMessage(windows_thread_id_.load().value(), WM_NULL, 0, 0);
}

///
///
inline auto hotkey::unregister_callback(const key key, const key_modifier mod) -> void
{
  assert(windows_thread_id_.load().has_value());
  register_queue.queue(
    [key, mod]()
    {
      const auto key_id = id_map_.at({key, mod});
      UnregisterHotKey(nullptr, key_id);
      id_map_.erase({key, mod});
      callback_map_.erase(key_id);
    });
  PostThreadMessage(windows_thread_id_.load().value(), WM_NULL, 0, 0);
}

///
///
inline auto hotkey::next_hotkey_id() -> int
{
  return hotkey_id_.fetch_add(1);
}

///
///
inline auto hotkey::get_message() -> void
{
  static MSG msg;
  if(listen_to_msg_)
  {
    if(const auto ret = GetMessage(&msg, nullptr, 0, 0); ret != 0)
    {
      if(ret == -1)
      {
        LOG_ERROR(log_channel, "Error getting message in {}", std::source_location::current().function_name());
      }
      else
      {
        while(!register_queue.empty())
        {
          register_queue.try_do_task();
        }
        TranslateMessage(&msg);
        message_handler(msg);
        DispatchMessage(&msg);
      }
      active_worker_type::queue_task(get_message);
    }
  }
}

///
///
inline auto hotkey::message_handler(const MSG& msg) -> void
{
  if(msg.message == WM_HOTKEY)
  {
    if(callback_map_.contains(msg.wParam))
    {
      auto callback = callback_map_.at(msg.wParam);
      app_framework::active_worker_main::run_task(std::move(callback));
    }
    else
    {
      LOG_ERROR(log_channel, "No callback registered for hotkey_id={}", msg.wParam);
    }
  }
}

} // namespace bibstd::system
