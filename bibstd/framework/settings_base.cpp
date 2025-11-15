#include "bibstd/framework/settings_base.hpp"

namespace bibstd::framework
{

///
///
settings_base::settings_base()
  : workflow_settings_{std::make_unique<workflow::workflow_settings>()}
{
}

///
///
settings_base::~settings_base() noexcept = default;

} // namespace bibstd::framework
