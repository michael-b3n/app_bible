#pragma once

#include "meta/for_each.hpp"
#include "util/contains.hpp"
#include "util/exception.hpp"
#include "util/non_owning_ptr.hpp"
#include "util/property_tree.hpp"
#include "util/setting_type_erased.hpp"

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
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
class workflow_settings final
{
public: // Typedefs
  template<util::underlying_setting_type_erased_type T>
  using setting_type_erased_non_owning_ptr_type = util::non_owning_ptr<util::setting_type_erased<T>>;
  template<util::underlying_setting_type_erased_type T>
  using setting_type_erased_uptr_type = std::unique_ptr<util::setting_type_erased<T>>;

  using setting_type_erased_non_owning_ptr_variant_type =
    meta::for_each_t<util::setting_type_erased_variant, setting_type_erased_non_owning_ptr_type>;
  using setting_type_erased_uptr_variant_type =
    meta::for_each_t<util::setting_type_erased_variant, setting_type_erased_uptr_type>;

  template<util::underlying_setting_type T>
  using setting_non_owning_ptr_type = util::non_owning_ptr<util::setting<T>>;

public: // Constants
  static constexpr std::string_view settings_file_name = "settings.xml";

public: // Static interface
  ///
  /// Get the default settings file path.
  /// \return settings file path
  ///
  static auto settings_file_path() -> const std::filesystem::path&;

  ///
  /// Access all created settings.
  /// \return list of all created settings
  ///
  static auto type_erased_settings() -> std::vector<setting_type_erased_non_owning_ptr_variant_type>;

public: // Structors
  workflow_settings(std::string&& parent);

public: // Modifiers
  ///
  /// Create a new setting. The setting will be owned by this workflow and has static lifetime.
  /// \param path Setting path must be unique
  /// \param name Name of setting
  /// \param default_value Default value of setting
  /// \param validator Setting validator
  /// \return the newly created setting
  ///
  template<util::underlying_setting_type T>
  [[nodiscard]] auto create_setting(
    const std::string& path,
    const std::string& name,
    T&& default_value,
    util::setting_validator<T>&& validator = std::make_shared<util::setting_validator_unbound>()
  ) -> setting_non_owning_ptr_type<T>;

private: // Typedefs
  struct setting_data final
  {
    std::string path;
    setting_type_erased_uptr_variant_type setting;
  };

private: // Variables
  const std::string parent_;
  inline static std::mutex settings_mtx_{};
  inline static std::vector<setting_data> settings_{};
  util::property_tree::sptr_type tree_{util::property_tree::create(settings_file_path())};
};

///
///
template<util::underlying_setting_type T>
auto workflow_settings::create_setting(
  const std::string& path, const std::string& name, T&& default_value, util::setting_validator<T>&& validator
) -> setting_non_owning_ptr_type<T>
{
  const auto setting = std::make_shared<util::setting<T>>(
    parent_,
    name,
    std::move(tree_->create_property(util::property_tree::path_type{path}, std::move(default_value))),
    std::move(validator)
  );
  const auto setting_ptr = setting.get();
  using underlying_setting_type_erased_type = util::setting_type_erased<util::setting_type_erased_type_from<T>>;

  const auto lock = std::lock_guard(settings_mtx_);
  const auto contains_path = util::contains(settings_, [&path](const auto& data) { return data.path == path; });
  if(contains_path)
  {
    const auto value_name = name.empty() ? "<unnamed>" : name;
    THROW_EXCEPTION(util::exception(std::format("setting already created: path=\"{}\", name=\"{}\"", path, value_name)));
  }
  settings_.emplace_back(setting_data{.path = path, .setting = std::make_unique<underlying_setting_type_erased_type>(setting)});
  return setting_ptr;
}

} // namespace bibstd::workflow
