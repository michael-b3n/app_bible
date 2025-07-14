#pragma once

#include "system/hotkey_common.hpp"
#include "util/scoped_guard.hpp"

#include <functional>

namespace bibstd::system
{

///
/// Generic hotkey class.
///
class hotkey final
{
public: // Typedefs
  using key = hotkey_common::key;
  using key_modifier = hotkey_common::key_modifier;

public: // Static modifiers
  ///
  /// Initialize hotkey registration.
  /// \return guard to uninitialize hotkey registration on destruction
  ///
  static auto init() -> util::scoped_guard;

  ///
  /// Register global callback to specified key and key modifier.
  /// \param key Key ID
  /// \param mod Key modifier ID
  /// \param callback Callback that shall be registered
  ///
  static auto register_callback(key key, key_modifier mod, std::function<void()>&& callback) -> void;

  ///
  /// Unregister global callback for specified key and key modifier.
  /// \param key Key ID
  /// \param mod Key modifier ID
  ///
  static auto unregister_callback(key key, key_modifier mod) -> void;
};

} // namespace bibstd::system
