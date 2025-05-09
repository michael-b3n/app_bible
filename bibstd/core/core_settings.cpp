#include "core/core_settings.hpp"

#include <algorithm>
#include <ranges>

namespace bibstd::core
{

///
///
core_settings::core_settings(std::string&& parent)
  : parent_{std::move(parent)}
{
}

///
///
auto core_settings::settings() -> std::vector<common_type::setting_non_owning_ptr_variant_type>
{
  const auto lock = std::lock_guard(mtx_);
  auto retval = std::vector<common_type::setting_non_owning_ptr_variant_type>(property_map_.size());
  const auto to_ptr = [](const auto& uptr)
  { return std::visit([](const auto& e) -> common_type::setting_non_owning_ptr_variant_type { return e.get(); }, uptr); };
  for(const auto [i, ptr] : property_map_ | std::views::values | std::views::transform(to_ptr) | std::views::enumerate)
  {
    retval.at(i) = ptr;
  }
  return retval;
}

} // namespace bibstd::core
