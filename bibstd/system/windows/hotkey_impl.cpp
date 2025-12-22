#include "bibstd/system/windows/hotkey_impl.hpp"
#include "bibstd/framework/task_queue.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/system/windows/win.hpp"
#include "bibstd/util/const_bimap.hpp"
#include "bibstd/util/enum.hpp"
#include "bibstd/util/log.hpp"

#include <algorithm>
#include <cassert>
#include <future>
#include <ranges>

namespace bibstd::system
{
namespace
{

///
/// Key map constant for Windows hotkeys.
///
constexpr auto key_map = util::const_bimap(
  std::pair{hotkey_common::key::vk_left_button, VK_LBUTTON},
  std::pair{hotkey_common::key::vk_right_button, VK_RBUTTON},
  std::pair{hotkey_common::key::vk_middle_button, VK_MBUTTON},
  std::pair{hotkey_common::key::vk_x1_button, VK_XBUTTON1},
  std::pair{hotkey_common::key::vk_x2_button, VK_XBUTTON2},
  std::pair{hotkey_common::key::vk_backspace, VK_BACK},
  std::pair{hotkey_common::key::vk_tab, VK_TAB},
  std::pair{hotkey_common::key::vk_clear, VK_CLEAR},
  std::pair{hotkey_common::key::vk_return, VK_RETURN},
  std::pair{hotkey_common::key::vk_pause, VK_PAUSE},
  std::pair{hotkey_common::key::vk_capital, VK_CAPITAL},
  std::pair{hotkey_common::key::vk_escape, VK_ESCAPE},
  std::pair{hotkey_common::key::vk_space, VK_SPACE},
  std::pair{hotkey_common::key::vk_page_up, VK_PRIOR},
  std::pair{hotkey_common::key::vk_page_down, VK_NEXT},
  std::pair{hotkey_common::key::vk_end, VK_END},
  std::pair{hotkey_common::key::vk_home, VK_HOME},
  std::pair{hotkey_common::key::vk_left, VK_LEFT},
  std::pair{hotkey_common::key::vk_up, VK_UP},
  std::pair{hotkey_common::key::vk_right, VK_RIGHT},
  std::pair{hotkey_common::key::vk_down, VK_DOWN},
  std::pair{hotkey_common::key::vk_insert, VK_INSERT},
  std::pair{hotkey_common::key::vk_delete, VK_DELETE},
  std::pair{hotkey_common::key::vk_0, 0x30},
  std::pair{hotkey_common::key::vk_1, 0x31},
  std::pair{hotkey_common::key::vk_2, 0x32},
  std::pair{hotkey_common::key::vk_3, 0x33},
  std::pair{hotkey_common::key::vk_4, 0x34},
  std::pair{hotkey_common::key::vk_5, 0x35},
  std::pair{hotkey_common::key::vk_6, 0x36},
  std::pair{hotkey_common::key::vk_7, 0x37},
  std::pair{hotkey_common::key::vk_8, 0x38},
  std::pair{hotkey_common::key::vk_9, 0x39},
  std::pair{hotkey_common::key::vk_a, 0x41},
  std::pair{hotkey_common::key::vk_b, 0x42},
  std::pair{hotkey_common::key::vk_c, 0x43},
  std::pair{hotkey_common::key::vk_d, 0x44},
  std::pair{hotkey_common::key::vk_e, 0x45},
  std::pair{hotkey_common::key::vk_f, 0x46},
  std::pair{hotkey_common::key::vk_g, 0x47},
  std::pair{hotkey_common::key::vk_h, 0x48},
  std::pair{hotkey_common::key::vk_i, 0x49},
  std::pair{hotkey_common::key::vk_j, 0x4A},
  std::pair{hotkey_common::key::vk_k, 0x4B},
  std::pair{hotkey_common::key::vk_l, 0x4C},
  std::pair{hotkey_common::key::vk_m, 0x4D},
  std::pair{hotkey_common::key::vk_n, 0x4E},
  std::pair{hotkey_common::key::vk_o, 0x4F},
  std::pair{hotkey_common::key::vk_p, 0x50},
  std::pair{hotkey_common::key::vk_q, 0x51},
  std::pair{hotkey_common::key::vk_r, 0x52},
  std::pair{hotkey_common::key::vk_s, 0x53},
  std::pair{hotkey_common::key::vk_t, 0x54},
  std::pair{hotkey_common::key::vk_u, 0x55},
  std::pair{hotkey_common::key::vk_v, 0x56},
  std::pair{hotkey_common::key::vk_w, 0x57},
  std::pair{hotkey_common::key::vk_x, 0x58},
  std::pair{hotkey_common::key::vk_y, 0x59},
  std::pair{hotkey_common::key::vk_z, 0x5A}
);

///
/// Key modifier map constant for Windows hotkeys.
///
constexpr auto key_modifier_map = util::const_bimap(
  std::pair{hotkey_common::key_modifier::alt, MOD_ALT},
  std::pair{hotkey_common::key_modifier::control, MOD_CONTROL},
  std::pair{hotkey_common::key_modifier::shift, MOD_SHIFT},
  std::pair{hotkey_common::key_modifier::alt_control, MOD_ALT | MOD_CONTROL},
  std::pair{hotkey_common::key_modifier::alt_shift, MOD_ALT | MOD_SHIFT},
  std::pair{hotkey_common::key_modifier::control_shift, MOD_CONTROL | MOD_SHIFT}
);

///
/// Generate a unique hotkey ID.
/// \return Unique hotkey ID
///
auto new_hotkey_id() -> int
{
  static std::atomic_int hotkey_id{0};
  return hotkey_id.fetch_add(1);
}

///
/// Create a static MSG object for message handling.
/// \return Reference to static MSG object
///
auto message() -> MSG&
{
  static MSG msg;
  return msg;
}

///
/// Message handler for Windows hotkey callbacks.
/// \param msg The message received from the Windows message queue
/// \param callback_map Map of hotkey IDs to their corresponding callback functions
///
auto message_handler(const MSG& msg, const std::map<int, std::function<void()>>& callback_map) -> void
{
  if(msg.message == WM_HOTKEY)
  {
    if(callback_map.contains(msg.wParam))
    {
      auto callback = callback_map.at(msg.wParam);
      framework::thread_pool::queue_task(std::move(callback));
    }
    else
    {
      LOG_ERROR("no callback registered for hotkey_id={}", msg.wParam);
    }
  }
}

} // namespace

///
///
auto hotkey_impl::init() -> util::scoped_guard
{
  worker_ = std::make_unique<framework::active_worker>();
  std::promise<void> promise;
  auto future = promise.get_future();
  worker_->queue_task(
    [&]()
    {
      windows_thread_id_ = GetCurrentThreadId();
      worker_->queue_task(
        [&]()
        {
          // PeekMessage to create the message queue for this thread.
          // This is needed such that PostThreadMessage can post messages to it.
          PeekMessage(&message(), nullptr, 0, 0, PM_REMOVE);
          promise.set_value();
        }
      );
      worker_->queue_task(get_message);
    }
  );
  future.get();
  return util::scoped_guard(
    []()
    {
      listen_to_msg_ = false;
      worker_->queue_task(
        [&]() { std::ranges::for_each(callback_map_ | std::views::keys, [](const auto id) { UnregisterHotKey(nullptr, id); }); }
      );
      PostThreadMessage(windows_thread_id_.load().value(), WM_QUIT, 0, 0);
      worker_.reset();
    }
  );
}

///
///
auto hotkey_impl::register_callback(
  const hotkey_common::key key, const hotkey_common::key_modifier mod, std::function<void()>&& callback
) -> void
{
  assert(windows_thread_id_.load().has_value());
  const auto hotkey_id = new_hotkey_id();
  register_queue_.queue(
    [key, mod, hotkey_id, c = std::forward<decltype(callback)>(callback)]() mutable
    {
      callback_map_[hotkey_id] = std::move(c);
      id_map_[std::pair{key, mod}] = hotkey_id;
      if(RegisterHotKey(nullptr, hotkey_id, key_modifier_map.at(mod), key_map.at(key)))
      {
        LOG_DEBUG("registered hotkey: id={}, key={}, modifier={}", hotkey_id, util::to_integral(key), util::to_integral(mod));
      }
      else
      {
        LOG_ERROR("failed to register hotkey: key={}, modifier={}", util::to_integral(key), util::to_integral(mod));
      }
    }
  );
  PostThreadMessage(windows_thread_id_.load().value(), WM_NULL, 0, 0);
}

///
///
auto hotkey_impl::unregister_callback(const hotkey_common::key key, const hotkey_common::key_modifier mod) -> void
{
  assert(windows_thread_id_.load().has_value());
  register_queue_.queue(
    [key, mod]()
    {
      const auto key_id = id_map_.at({key, mod});
      UnregisterHotKey(nullptr, key_id);
      id_map_.erase({key, mod});
      callback_map_.erase(key_id);
    }
  );
  PostThreadMessage(windows_thread_id_.load().value(), WM_NULL, 0, 0);
}

///
///
auto hotkey_impl::get_message() -> void
{
  if(listen_to_msg_)
  {
    if(const auto ret = GetMessage(&message(), nullptr, 0, 0); ret != 0)
    {
      if(ret == -1)
      {
        LOG_ERROR("error getting message in {}", std::source_location::current().function_name());
      }
      else
      {
        while(!register_queue_.empty())
        {
          register_queue_.try_do_task();
        }
        TranslateMessage(&message());
        message_handler(message(), callback_map_);
        DispatchMessage(&message());
      }
      worker_->queue_task(get_message);
    }
  }
}

} // namespace bibstd::system
