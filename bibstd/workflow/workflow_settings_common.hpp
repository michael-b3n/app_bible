#pragma once

#include "meta/for_each.hpp"
#include "system/filesystem.hpp"
#include "util/non_owning_ptr.hpp"
#include "util/setting_type_erased.hpp"

#include <filesystem>
#include <string_view>

namespace bibstd::workflow
{

///
/// Workflow setting common. This helper struct contains helper types for workflow settings.
///
struct workflow_settings_common final
{
  // Typedefs
  template<util::erased_setting_type T>
  using default_setting_non_owning_ptr_type = util::non_owning_ptr<util::setting_type_erased<T>>;
  template<util::erased_setting_type T>
  using default_setting_uptr_type = std::unique_ptr<util::setting_type_erased<T>>;

  using default_setting_non_owning_ptr_variant_type =
    meta::for_each_t<util::default_setting_variant, default_setting_non_owning_ptr_type>;
  using default_setting_uptr_variant_type = meta::for_each_t<util::default_setting_variant, default_setting_uptr_type>;

  template<util::underlying_setting_type T>
  using setting_non_owning_ptr_type = util::non_owning_ptr<util::setting<T>>;

  // Constants
  static constexpr std::string_view settings_file_name = "settings.xml";

  // Helper functions
  ///
  /// Get the default settings file path.
  /// \return settings file path
  ///
  static inline auto settings_file_path() -> const std::filesystem::path&;
};

///
///
auto workflow_settings_common::settings_file_path() -> const std::filesystem::path&
{
  static const std::filesystem::path path{system::filesystem::local_data_folder() / settings_file_name};
  return path;
}

} // namespace bibstd::workflow
