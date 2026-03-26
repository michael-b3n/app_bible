#pragma once

#include "bibstd/bible/common.hpp"
#include "bibstd/bible/reference_range.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <memory>
#include <vector>

namespace bibstd::core
{
// Forward declarations
class core_lookup_bibleserver;
} // namespace bibstd::core
namespace bibstd::workflow
{

// Forward declarations
class workflow_bible_ref_lookup;

namespace detail
{

///
/// Available signal IDs.
///
enum class workflow_bible_ref_lookup_signal_id
{
  ended
};

///
/// Start parameters for workflow bible reference lookup.
///
struct workflow_bible_ref_lookup_start_params final
{
  std::vector<bible::reference_range> references;
};

///
/// Expected result type for workflow bible reference lookup.
///
using workflow_bible_ref_lookup_expected_result_type = void; // TODO: Define result type

///
/// Base type definition.
///
using workflow_bible_ref_lookup_base_type = workflow_base<
  workflow_bible_ref_lookup,                       // derived workflow
  workflow_bible_ref_lookup_start_params,          // start params type,
  workflow_bible_ref_lookup_expected_result_type>; // expected result type

} // namespace detail

///
/// Settings corresponding to workflow bible reference lookup.
///
class workflow_bible_ref_lookup_settings final : public framework::settings_base
{
public: // Structors
  workflow_bible_ref_lookup_settings();
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
  : public detail::workflow_bible_ref_lookup_base_type
  , public framework::settings_owner<workflow_bible_ref_lookup_settings>
  , public signal::adapter<signal::named_signal<
      detail::workflow_bible_ref_lookup_signal_id::ended,
      signal::signal_type<void(const detail::workflow_bible_ref_lookup_base_type::result_params&)>>>
{
public: // Typedefs
  using params_type = detail::workflow_bible_ref_lookup_base_type::params_type;
  using result_type = detail::workflow_bible_ref_lookup_base_type::result_type;

public: // Structors
  workflow_bible_ref_lookup();
  ~workflow_bible_ref_lookup() noexcept;

public: // Modifiers
  ///
  /// Lookup bible references.
  /// \param params Start parameters for the workflow
  ///
  auto lookup(const start_params& params) -> void;

private: // Variables
  const framework::thread_pool::strand_id_type strand_id_{framework::thread_pool::strand_id()};
  const std::unique_ptr<core::core_lookup_bibleserver> core_lookup_bibleserver_;
};

} // namespace bibstd::workflow
