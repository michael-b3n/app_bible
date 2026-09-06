#include "bibstd/system/windows/hotkey_impl.hpp"
#include "bibstd/framework/task_queue.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/util/enum.hpp"
#include "bibstd/util/log.hpp"

#include "bibstd/system/windows/win.hpp"

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <future>
#include <mutex>
#include <ranges>

namespace bibstd::system
{
namespace
{

///
/// Key map constant for Windows hotkeys.
///
constexpr auto key_map = util::make_const_bimap<hotkey_common::key, unsigned int>({
  {  hotkey_common::key::vk_left_button,  VK_LBUTTON},
  { hotkey_common::key::vk_right_button,  VK_RBUTTON},
  {hotkey_common::key::vk_middle_button,  VK_MBUTTON},
  {    hotkey_common::key::vk_x1_button, VK_XBUTTON1},
  {    hotkey_common::key::vk_x2_button, VK_XBUTTON2},
  {    hotkey_common::key::vk_backspace,     VK_BACK},
  {          hotkey_common::key::vk_tab,      VK_TAB},
  {        hotkey_common::key::vk_clear,    VK_CLEAR},
  {       hotkey_common::key::vk_return,   VK_RETURN},
  {        hotkey_common::key::vk_pause,    VK_PAUSE},
  {      hotkey_common::key::vk_capital,  VK_CAPITAL},
  {       hotkey_common::key::vk_escape,   VK_ESCAPE},
  {        hotkey_common::key::vk_space,    VK_SPACE},
  {      hotkey_common::key::vk_page_up,    VK_PRIOR},
  {    hotkey_common::key::vk_page_down,     VK_NEXT},
  {          hotkey_common::key::vk_end,      VK_END},
  {         hotkey_common::key::vk_home,     VK_HOME},
  {         hotkey_common::key::vk_left,     VK_LEFT},
  {           hotkey_common::key::vk_up,       VK_UP},
  {        hotkey_common::key::vk_right,    VK_RIGHT},
  {         hotkey_common::key::vk_down,     VK_DOWN},
  {       hotkey_common::key::vk_insert,   VK_INSERT},
  {       hotkey_common::key::vk_delete,   VK_DELETE},
  {            hotkey_common::key::vk_0,        0x30},
  {            hotkey_common::key::vk_1,        0x31},
  {            hotkey_common::key::vk_2,        0x32},
  {            hotkey_common::key::vk_3,        0x33},
  {            hotkey_common::key::vk_4,        0x34},
  {            hotkey_common::key::vk_5,        0x35},
  {            hotkey_common::key::vk_6,        0x36},
  {            hotkey_common::key::vk_7,        0x37},
  {            hotkey_common::key::vk_8,        0x38},
  {            hotkey_common::key::vk_9,        0x39},
  {            hotkey_common::key::vk_a,        0x41},
  {            hotkey_common::key::vk_b,        0x42},
  {            hotkey_common::key::vk_c,        0x43},
  {            hotkey_common::key::vk_d,        0x44},
  {            hotkey_common::key::vk_e,        0x45},
  {            hotkey_common::key::vk_f,        0x46},
  {            hotkey_common::key::vk_g,        0x47},
  {            hotkey_common::key::vk_h,        0x48},
  {            hotkey_common::key::vk_i,        0x49},
  {            hotkey_common::key::vk_j,        0x4A},
  {            hotkey_common::key::vk_k,        0x4B},
  {            hotkey_common::key::vk_l,        0x4C},
  {            hotkey_common::key::vk_m,        0x4D},
  {            hotkey_common::key::vk_n,        0x4E},
  {            hotkey_common::key::vk_o,        0x4F},
  {            hotkey_common::key::vk_p,        0x50},
  {            hotkey_common::key::vk_q,        0x51},
  {            hotkey_common::key::vk_r,        0x52},
  {            hotkey_common::key::vk_s,        0x53},
  {            hotkey_common::key::vk_t,        0x54},
  {            hotkey_common::key::vk_u,        0x55},
  {            hotkey_common::key::vk_v,        0x56},
  {            hotkey_common::key::vk_w,        0x57},
  {            hotkey_common::key::vk_x,        0x58},
  {            hotkey_common::key::vk_y,        0x59},
  {            hotkey_common::key::vk_z,        0x5A}
});

///
/// Key modifier map constant for Windows hotkeys.
///
constexpr auto key_modifier_map = util::make_const_bimap<hotkey_common::key_modifier, unsigned int>({
  {          hotkey_common::key_modifier::alt,                 MOD_ALT},
  {      hotkey_common::key_modifier::control,             MOD_CONTROL},
  {        hotkey_common::key_modifier::shift,               MOD_SHIFT},
  {  hotkey_common::key_modifier::alt_control,   MOD_ALT | MOD_CONTROL},
  {    hotkey_common::key_modifier::alt_shift,     MOD_ALT | MOD_SHIFT},
  {hotkey_common::key_modifier::control_shift, MOD_CONTROL | MOD_SHIFT}
});

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
auto hotkey_impl::init() -> util::shared_scope_guard
{
  static std::mutex mtx;
  static util::shared_scope_guard::creator guard_creator{};
  static util::shared_scope_guard thread_pool_guard;
  static std::condition_variable cv_init;

  auto lock = std::unique_lock{mtx};
  auto guard = guard_creator.create(
    []()
    {
      const auto lock = std::scoped_lock{mtx};
      listen_to_msg_ = false;
      std::promise<void> promise;
      auto future = promise.get_future();
      worker_->queue_task(
        [&]()
        {
          std::ranges::for_each(callback_map_ | std::views::keys, [](const auto id) { UnregisterHotKey(nullptr, id); });
          promise.set_value();
        }
      );
      PostThreadMessage(windows_thread_id_.load().value(), WM_QUIT, 0, 0);
      future.get();
      worker_.reset();
      thread_pool_guard.reset();
      cv_init.notify_all();
    }
  );
  if(guard.is_initial_instance())
  {
    if(worker_)
    {
      cv_init.wait(lock, []() { return !worker_; });
    }
    thread_pool_guard = framework::thread_pool::init();
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
  }
  return guard;
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
      if(id_map_.contains({key, mod}))
      {
        const auto key_id = id_map_.at({key, mod});
        UnregisterHotKey(nullptr, key_id);
        id_map_.erase({key, mod});
        callback_map_.erase(key_id);
      }
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
