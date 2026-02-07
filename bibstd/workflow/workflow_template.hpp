#pragma once

#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <stop_token>

namespace bibstd::workflow
{

// Forward declarations
class workflow_template;

namespace detail
{

///
/// Available signal IDs.
///
enum class workflow_template_signal_id
{
  ended
};

///
/// Start parameters for workflow template.
///
struct workflow_template_start_params final
{
  int value{0}; /*some type*/
};

///
/// Expected result type for workflow template.
///
using workflow_template_expected_result_type = float /*some type*/;

///
/// Base type definition.
///
using workflow_template_base_type = workflow_base<
  workflow_template,                       // derived workflow
  workflow_template_start_params,          // start params type,
  workflow_template_expected_result_type>; // expected result type

} // namespace detail

///
/// Settings corresponding to workflow template.
///
class workflow_template_settings final : public framework::settings_base
{
public: // Structors
  workflow_template_settings();
  ~workflow_template_settings() noexcept = default;

public: // Variables
  const setting_type<std::optional<std::string>> text;
  const setting_type<bool> flag;
};

///
/// Workflow template. This is a template for creating new workflows.
/// Signal IDs to connect to:
/// - ended: Emitted when the workflow ends. Slots receive the result parameters `result_type`.
///
class workflow_template final
  : public detail::workflow_template_base_type
  , public framework::settings_owner<workflow_template_settings>
  , public signal::adapter<signal::named_signal<
      detail::workflow_template_signal_id::ended,
      signal::signal_type<void(const detail::workflow_template_base_type::result_type&)>>>
{
public: // Typedefs
  using start_params = detail::workflow_template_base_type::start_params;
  using result_type = detail::workflow_template_base_type::result_type;

public: // Structors
  workflow_template();
  ~workflow_template() noexcept;

public: // Modifiers
  ///
  /// Start the workflow template.
  /// \param params Start parameters for the workflow
  /// \return start result containing a process ID and a stop source for stopping the workflow
  ///
  auto start(const start_params& params) -> std::stop_source;

private: // Variables
  const framework::thread_pool::strand_id_type strand_id_{framework::thread_pool::strand_id()};
};

} // namespace bibstd::workflow
