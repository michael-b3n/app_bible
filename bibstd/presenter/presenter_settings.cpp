#include "bibstd/presenter/presenter_settings.hpp"

namespace bibstd::presenter
{

///
///
auto presenter_settings::type_erased_settings() -> std::vector<setting_type_erased_non_owning_ptr_variant_type>
{
  return workflow::workflow_settings::type_erased_settings();
}

///
///
auto presenter_settings::type_erased_setting(const std::string& path)
  -> std::optional<setting_type_erased_non_owning_ptr_variant_type>
{
  return workflow::workflow_settings::type_erased_setting(path);
}

} // namespace bibstd::presenter
