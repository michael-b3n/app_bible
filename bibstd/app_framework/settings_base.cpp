#include "app_framework/settings_base.hpp"
#include "core/core_settings.hpp"

namespace bibstd::app_framework
{

///
///
settings_base::settings_base(std::string&& parent)
  : core_settings_{std::make_unique<core::core_settings>(std::move(parent))}
{
}

///
///
settings_base::~settings_base() noexcept = default;

} // namespace bibstd::app_framework
