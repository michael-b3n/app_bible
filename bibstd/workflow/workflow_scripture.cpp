#include "bibstd/workflow/workflow_scripture.hpp"
#include "bibstd/core/core_scripture_store.hpp"

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_scripture_settings::workflow_scripture_settings()
  // TODO: Initialize settings
// clang-format on
{
}

///
///
workflow_scripture::workflow_scripture()
  : core_scripture_store_(std::make_unique<core::core_scripture_store>())
{
}

///
///
workflow_scripture::~workflow_scripture() noexcept = default;

///
///
auto workflow_scripture::start(const start_params& params) -> std::stop_source
{
  const std::stop_source stop_source;
  // TODO: Implement workflow logic
  emit<signal_id::ended>(result_type{params.process_id(), std::unexpected{unexpected_result::failure}});
  return stop_source;
}

} // namespace bibstd::workflow
