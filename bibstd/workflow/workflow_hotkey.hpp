#pragma once

#include "bibstd/signal/common.hpp"
#include "bibstd/system/hotkey_common.hpp"
#include "bibstd/util/scope_guard.hpp"
#include "bibstd/workflow/workflow_base.hpp"

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
private: // Typedefs
  ///
  /// Hotkey parameters to assign a registered callback to a specific hotkey.
  ///
  struct assign_params_t final
  {
    // Typedefs
    using path_type = std::string;
    using hotkey_type = system::hotkey_common;

    // Constructors
    assign_params_t(path_type path, hotkey_type::key_modifier modifier, hotkey_type::key key);

    // Variables
    path_type path;
    hotkey_type::key_modifier modifier;
    hotkey_type::key key;
  };

public: // Typedefs
  using path_type = assign_params_t::path_type;
  using assign_params = framework::process_params<assign_params_t>;
  using shared_sig_type = std::shared_ptr<signal::signal_type<void()>>;

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
  auto assign_hotkey(const assign_params& params) -> bool;

  ///
  /// Register a callback to a specific path.
  /// \param path The path to register the callback
  /// \return The shared signal associated with the path
  ///
  [[nodiscard]] auto register_callback(const path_type& path) -> shared_sig_type;

private: // Variables
  const util::shared_scope_guard thread_pool_guard_;
  const util::shared_scope_guard hotkey_guard_;
  mutable std::mutex mtx_;
  std::unordered_map<path_type, shared_sig_type> shared_sigs_;
};

} // namespace bibstd::workflow
