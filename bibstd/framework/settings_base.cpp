#include "bibstd/framework/settings_base.hpp"

namespace bibstd::framework
{

///
///
settings_base::settings_base(std::string&& parent)
  : workflow_settings_{std::make_unique<workflow::workflow_settings>(std::move(parent))}
{
}

///
///
settings_base::~settings_base() noexcept = default;

} // namespace bibstd::framework
