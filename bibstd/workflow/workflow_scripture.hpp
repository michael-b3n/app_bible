#pragma once

#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <memory>
#include <stop_token>

namespace bibstd::core
{
// Forward declarations
class core_scripture_store;
} // namespace bibstd::core
namespace bibstd::workflow
{

// Forward declarations
class workflow_scripture;

namespace detail
{

///
/// Available signal IDs.
///
enum class workflow_scripture_signal_id
{
  ended
};

///
/// Start parameters for workflow scripture.
///
struct workflow_scripture_start_params final
{
  // TODO: Add start parameters
};

///
/// Expected result type for workflow scripture.
///
using workflow_scripture_expected_result_type = void; // TODO: Define result type

///
/// Base type definition.
///
using workflow_scripture_base_type = workflow_base<
  workflow_scripture,                       // derived workflow
  workflow_scripture_start_params,          // start params type,
  workflow_scripture_expected_result_type>; // expected result type

} // namespace detail

///
/// Settings corresponding to workflow scripture.
///
class workflow_scripture_settings final : public framework::settings_base
{
public: // Structors
  workflow_scripture_settings();
  ~workflow_scripture_settings() noexcept = default;

public: // Variables
        // TODO: Add settings
};

///
/// Workflow for scripture.
/// Signal IDs to connect to:
/// - ended: Emitted when the workflow ends. Slots receive the result parameters `result_type`.
///
class workflow_scripture final
  : public detail::workflow_scripture_base_type
  , public framework::settings_owner<workflow_scripture_settings>
  , public signal::adapter<signal::named_signal<
      detail::workflow_scripture_signal_id::ended,
      signal::signal_type<void(const detail::workflow_scripture_base_type::result_type&)>>>
{
public: // Typedefs
  using start_params = detail::workflow_scripture_base_type::start_params;
  using result_type = detail::workflow_scripture_base_type::result_type;

public: // Structors
  workflow_scripture();
  ~workflow_scripture() noexcept;

public: // Modifiers
  ///
  /// Start the workflow scripture.
  /// \param params Start parameters for the workflow
  /// \return start result containing a process ID and a stop source for stopping the workflow
  ///
  auto start(const start_params& params) -> std::stop_source;

private: // Variables
  const framework::thread_pool::strand_id_type strand_id_{framework::thread_pool::strand_id()};
  const std::unique_ptr<core::core_scripture_store> core_scripture_store_;
};

} // namespace bibstd::workflow
