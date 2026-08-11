#include "bibstd/workflow/workflow_settings.hpp"
#include "bibstd/system/filesystem.hpp"
#include "bibstd/util/string.hpp"

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
auto workflow_settings::split_path(const std::string_view path) -> std::vector<std::string>
{
  auto segments = util::string::split(path, path_separator);
  std::erase_if(segments, [](const auto& segment) { return segment.empty(); });
  return segments;
}

///
///
auto workflow_settings::type_erased_settings() const -> std::vector<setting_data>
{
  const auto lock = std::lock_guard(mtx_);
  auto retval = std::vector<setting_data>(settings_.size());
  const auto to_setting_data = [](const auto& data)
  {
    return setting_data{
      data.path,
      std::visit([](const auto& e) -> setting_type_erased_non_owning_ptr_variant_type { return e.get(); }, data.setting)
    };
  };
  for(const auto [i, d] : settings_ | std::views::transform(to_setting_data) | std::views::enumerate)
  {
    retval.at(i) = d;
  }
  return retval;
}

///
///
auto workflow_settings::type_erased_setting(const std::string& path) const
  -> std::optional<setting_type_erased_non_owning_ptr_variant_type>
{
  const auto lock = std::lock_guard(mtx_);
  const auto it = std::ranges::find_if(settings_, [&path](const auto& data) { return data.path == path; });
  if(it != std::ranges::cend(settings_))
  {
    return std::visit([](const auto& e) -> setting_type_erased_non_owning_ptr_variant_type { return e.get(); }, it->setting);
  }
  return std::nullopt;
}

} // namespace bibstd::workflow
