#include "bibstd/workflow/workflow_template.hpp"

namespace bibstd::workflow
{

///
///
workflow_template_settings::workflow_template_settings(std::shared_ptr<workflow_settings> workflow_settings)
  : framework::settings_base{std::move(workflow_settings)}
  , text(workflow_settings_->create_setting("template.text", setting_value_t<decltype(text)>{"default value"}))
  , flag(workflow_settings_->create_setting("template.flag", false))
{
}

///
///
workflow_template::workflow_template(std::shared_ptr<workflow_settings> workflow_settings)
  : workflow_base{std::move(workflow_settings)}
{
}

///
///
workflow_template::~workflow_template() noexcept = default;

///
///
auto workflow_template::start(const params& params) -> result
{
  // do something
  return return_failure;
}

} // namespace bibstd::workflow
