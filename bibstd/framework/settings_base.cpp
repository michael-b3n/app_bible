#include "bibstd/framework/settings_base.hpp"

namespace bibstd::framework
{

///
///
settings_base::settings_base(std::shared_ptr<workflow::workflow_settings> workflow_settings)
  : workflow_settings_{std::move(workflow_settings)}
{
}

///
///
settings_base::~settings_base() noexcept = default;

} // namespace bibstd::framework
