#include "bibstd/workflow/workflow_settings.hpp"
#include "bibstd/system/filesystem.hpp"

#include <algorithm>
#include <ranges>

namespace bibstd::workflow
{

///
///
auto workflow_settings::settings_file_path() -> const std::filesystem::path&
{
  static const std::filesystem::path path{system::filesystem::local_data_folder() / settings_file_name};
  return path;
}

///
///
auto workflow_settings::type_erased_settings() -> std::vector<setting_type_erased_non_owning_ptr_variant_type>
{
  const auto lock = std::lock_guard(settings_mtx_);
  auto retval = std::vector<setting_type_erased_non_owning_ptr_variant_type>(settings_.size());
  const auto to_ptr = [](const auto& data)
  { return std::visit([](const auto& e) -> decltype(retval)::value_type { return e.get(); }, data.setting); };
  for(const auto [i, ptr] : settings_ | std::views::transform(to_ptr) | std::views::enumerate)
  {
    retval.at(i) = ptr;
  }
  return retval;
}

///
///
workflow_settings::workflow_settings(std::string&& parent)
  : parent_{std::move(parent)}
{
}

} // namespace bibstd::workflow
