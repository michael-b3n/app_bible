#pragma once

#include "bibstd/framework/active_worker.hpp"
#include "bibstd/system/hotkey_common.hpp"
#include "bibstd/util/scope_guard.hpp"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace bibstd::system
{

///
/// Hotkey implementation class for the windows OS.
///
class hotkey_impl final
{
public: // Static modifiers
  ///
  /// \see hotkey::init
  ///
  static auto init() -> util::shared_scope_guard;

  ///
  /// \see hotkey::register_callback
  ///
  static auto register_callback(hotkey_common::key key, hotkey_common::key_modifier mod, std::function<void()>&& callback)
    -> void;

  ///
  /// \see hotkey::unregister_callback
  ///
  static auto unregister_callback(hotkey_common::key key, hotkey_common::key_modifier mod) -> void;

private: // Static helpers
  static auto get_message() -> void;

private:
  inline static std::unique_ptr<framework::active_worker> worker_{};
  inline static std::atomic_bool listen_to_msg_{true};
  inline static std::atomic<std::optional<unsigned long /*WORD*/>> windows_thread_id_{std::nullopt};
  inline static framework::task_queue register_queue_{};
  inline static std::map<int, std::function<void()>> callback_map_{};
  inline static std::map<std::pair<hotkey_common::key, hotkey_common::key_modifier>, int> id_map_{};
};

} // namespace bibstd::system
