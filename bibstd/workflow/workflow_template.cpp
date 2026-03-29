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
auto workflow_template::start(const process_params& params) -> process_result
{
  // do something
  return return_failure;
}

} // namespace bibstd::workflow
