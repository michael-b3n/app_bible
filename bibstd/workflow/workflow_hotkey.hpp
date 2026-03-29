#pragma once

#include "bibstd/system/hotkey_common.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace bibstd::workflow
{

///
/// Workflow hotkey. This workflow manages the registration of a hotkey and the corresponding callback.
///
class workflow_hotkey final : public workflow_base<workflow_hotkey>
{
public: // Typedefs
  ///
  /// Register parameters to register a callback to a specific path.
  ///
  struct register_params final
  {
    // Typedefs
    using callback_type = std::function<void()>;

    // Constructors
    register_params(std::string&& path, callback_type&& callback);

    // Variables
    std::string path;
    callback_type callback;
  };

  ///
  /// Hotkey parameters to assign a registered callback to a specific hotkey.
  ///
  struct hotkey_params final
  {
    // Typedefs
    using hotkey_type = system::hotkey_common;

    // Constructors
    hotkey_params(std::string&& path, hotkey_type::key_modifier modifier, hotkey_type::key key);

    // Variables
    std::string path;
    hotkey_type::key_modifier modifier;
    hotkey_type::key key;
  };

public: // Structors
  workflow_hotkey();
  ~workflow_hotkey() noexcept;

public: // Accessors
  ///
  /// Get the registered callback paths.
  /// \return registered callback paths
  ///
  [[nodiscard]] auto available_callbacks() const -> std::vector<std::string>;

public: // Modifiers
  ///
  /// Assign a registered callback to a specific hotkey.
  /// \param params Hotkey parameters containing the path, modifier, and key
  /// \return true if the hotkey was successfully assigned, false otherwise
  ///
  auto assign_hotkey(const hotkey_params& params) -> bool;

  ///
  /// Register a callback to a specific path.
  /// \param params Register parameters containing the path and callback
  ///
  auto register_callback(const register_params& params) -> void;

private: // Typedefs
  struct shutdown_flag final
  {};

private: // Variables
  mutable std::mutex mtx_;
  std::unordered_map<std::string, register_params::callback_type> callbacks_;
};

} // namespace bibstd::workflow
