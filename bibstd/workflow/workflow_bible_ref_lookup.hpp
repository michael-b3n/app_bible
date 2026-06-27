#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/reference_range.hpp"
#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/signal/common.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <memory>
#include <vector>

// Forward declarations
namespace bibstd::core
{
class core_lookup_bibleserver;
} // namespace bibstd::core

namespace bibstd::workflow
{

///
/// Signals for workflow bible reference lookup.
///
struct workflow_bible_ref_lookup_sigs final
{
  signal::signal_type<void(framework::process_id_type)> ended;
};

///
/// Settings corresponding to workflow bible reference lookup.
///
class workflow_bible_ref_lookup_settings final : public framework::settings_base
{
public: // Structors
  workflow_bible_ref_lookup_settings(std::shared_ptr<workflow_settings> workflow_settings);
  ~workflow_bible_ref_lookup_settings() noexcept = default;

public: // Variables
  const setting_type<std::vector<bible::translation>> translations;
};

///
/// Workflow for bible reference lookup.
/// Signal IDs to connect to:
/// - ended: Emitted when the workflow ends. Slots receive the result parameters `result_type`.
///
class workflow_bible_ref_lookup final
  : public workflow_base<workflow_bible_ref_lookup_settings>
  , public signal::adapter<workflow_bible_ref_lookup_sigs>
{
  // Typedefs
  struct params_t final
  {
    std::vector<bible::reference_range> references;
  };

  // Variables
  const util::shared_scope_guard thread_pool_guard_;
  const framework::thread_pool::strand_id_type strand_id_{framework::thread_pool::strand_id()};
  const std::unique_ptr<core::core_lookup_bibleserver> core_lookup_bibleserver_;

public: // Typedefs
  using params = framework::process_params<params_t>;

public: // Structors
  workflow_bible_ref_lookup(std::shared_ptr<workflow_settings> workflow_settings);
  ~workflow_bible_ref_lookup() noexcept;

public: // Modifiers
  ///
  /// Lookup bible references.
  /// \param params Start parameters for the workflow
  ///
  auto lookup(const params& params) -> void;
};

} // namespace bibstd::workflow
