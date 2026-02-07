#include "bibstd/workflow/workflow_template.hpp"

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_template_settings::workflow_template_settings()
  : text(workflow_settings_->create_setting("template.text", std::optional<std::string>{"default value"}))
  , flag(workflow_settings_->create_setting("template.flag", false))
// clang-format on
{
}

///
///
workflow_template::workflow_template() = default;

///
///
workflow_template::~workflow_template() noexcept = default;

///
///
auto workflow_template::start(const start_params& params) -> std::stop_source
{
  const std::stop_source stop_source;
  // do something
  emit<signal_id::ended>(result_type{params.process_id(), std::unexpected{unexpected_result::failure}});
  return stop_source;
}

} // namespace bibstd::workflow
