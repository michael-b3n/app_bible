#pragma once

#include "bibstd/framework/property_tree.hpp"
#include "bibstd/framework/setting_type_erased.hpp"
#include "bibstd/meta/for_each.hpp"
#include "bibstd/util/contains.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/non_owning_ptr.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>

namespace bibstd::workflow
{

///
/// Workflow setting. This class owns setting objects. This workflow is thread safe and static.
/// There can be multiple workflows of the same type but all settings are shared.
/// \tparam ...Args List of all settings types.
///
class workflow_settings final : public workflow_base<workflow_settings>
{
  // Typedefs
  template<framework::underlying_setting_type_erased_type T>
  using setting_type_erased_non_owning_ptr_type = util::non_owning_ptr<framework::setting_type_erased<T>>;
  template<framework::underlying_setting_type_erased_type T>
  using setting_type_erased_uptr_type = std::unique_ptr<framework::setting_type_erased<T>>;
  using setting_type_erased_uptr_variant_type =
    meta::for_each_t<framework::setting_type_erased_variant, setting_type_erased_uptr_type>;

public: // Typedefs
  using setting_type_erased_non_owning_ptr_variant_type =
    meta::for_each_t<framework::setting_type_erased_variant, setting_type_erased_non_owning_ptr_type>;

  template<framework::underlying_setting_type T>
  using setting_non_owning_ptr_type = util::non_owning_ptr<framework::setting<T>>;

public: // Constants
  static constexpr std::string_view settings_file_name = "settings.xml";

public: // Static interface
  ///
  /// Get the default settings file path.
  /// \return settings file path
  ///
  [[nodiscard]] static auto settings_file_path() -> const std::filesystem::path&;

  ///
  /// Access all created settings.
  /// \return list of all created settings
  ///
  [[nodiscard]] static auto type_erased_settings() -> std::vector<setting_type_erased_non_owning_ptr_variant_type>;

  ///
  /// Access a type erased setting for the specified path.
  /// \param path Setting path
  /// \return type erased setting or nullopt if no setting with the specified path exists
  ///
  [[nodiscard]] static auto type_erased_setting(const std::string& path)
    -> std::optional<setting_type_erased_non_owning_ptr_variant_type>;

public: // Structors
  workflow_settings() = default;

public: // Modifiers
  ///
  /// Create a new setting. The setting will be owned by this workflow and has static lifetime.
  /// \param path Setting path must be unique
  /// \param name Name of setting
  /// \param default_value Default value of setting
  /// \param validator Setting validator
  /// \return the newly created setting
  ///
  template<framework::underlying_setting_type T>
  [[nodiscard]] auto create_setting(
    const std::string& path,
    T&& default_value,
    framework::setting_validator<T>&& validator = std::make_shared<framework::setting_validator_unbound>()
  ) -> setting_non_owning_ptr_type<T>;

private: // Typedefs
  struct setting_data final
  {
    std::string path;
    setting_type_erased_uptr_variant_type setting;
  };

private: // Variables
  inline static std::mutex settings_mtx_{};
  inline static std::vector<setting_data> settings_{};
  const framework::property_tree::sptr_type tree_{framework::property_tree::create(settings_file_path())};
};

///
///
template<framework::underlying_setting_type T>
auto workflow_settings::create_setting(const std::string& path, T&& default_value, framework::setting_validator<T>&& validator)
  -> setting_non_owning_ptr_type<T>
{
  const auto setting = std::make_shared<framework::setting<T>>(
    path,
    std::move(tree_->create_property(framework::property_tree::path_type{path}, std::move(default_value))),
    std::move(validator)
  );
  const auto setting_ptr = setting.get();
  using underlying_setting_type_erased_type = framework::setting_type_erased<framework::setting_type_erased_type_from<T>>;

  const auto lock = std::lock_guard(settings_mtx_);
  const auto contains_path = util::contains(settings_, [&path](const auto& data) { return data.path == path; });
  if(contains_path)
  {
    throw util::exception(std::format("setting already created: path=\"{}\"", path));
  }
  settings_.emplace_back(setting_data{.path = path, .setting = std::make_unique<underlying_setting_type_erased_type>(setting)});
  return setting_ptr;
}

} // namespace bibstd::workflow
