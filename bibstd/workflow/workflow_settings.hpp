#pragma once

#include "util/exception.hpp"
#include "util/property_tree.hpp"
#include "util/setting_type_erased.hpp"
#include "workflow/workflow_settings_common.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>

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
  using common_type = workflow_settings_common;

public: // Structors
  workflow_settings(std::string&& parent);

public: // Accessors
  ///
  /// Access all created settings of this workflow.
  /// \return list of all created settings
  ///
  auto type_erased_settings() -> std::vector<common_type::default_setting_non_owning_ptr_variant_type>;

public: // Modifiers
  ///
  /// Create a new setting owned by this workflow.
  /// \param value_path Value path must be unique
  /// \param value_name Name of setting
  /// \param default_value Default value of setting
  /// \param validator Setting validator
  /// \return the newly created setting
  ///
  template<util::underlying_setting_type T>
  [[nodiscard]] auto create_setting(
    const std::string& value_path,
    const std::string& value_name,
    T&& default_value,
    util::setting_validator<T>&& validator = util::setting_validator_unbound{}
  ) -> common_type::setting_non_owning_ptr_type<T>;

private: // Variables
  const std::string parent_;
  mutable std::mutex mtx_;
  inline static std::map<std::string, common_type::default_setting_uptr_variant_type> setting_map_{};
  util::property_tree::sptr_type tree_{util::property_tree::create(common_type::settings_file_path())};
};

///
///
template<util::underlying_setting_type T>
auto workflow_settings::create_setting(
  const std::string& value_path, const std::string& value_name, T&& default_value, util::setting_validator<T>&& validator
) -> common_type::setting_non_owning_ptr_type<T>
{
  const auto lock = std::lock_guard(mtx_);
  auto setting = std::make_shared<util::setting<T>>(
    parent_,
    value_name,
    std::move(tree_->create_property(util::property_tree::path_type{value_path}, std::move(default_value))),
    std::move(validator)
  );
  const auto setting_ptr = setting.get();
  using erased_setting_type = util::setting_type_erased<util::erased_setting_type_from<T>>;
  const auto [_, inserted] = setting_map_.emplace(value_path, std::make_unique<erased_setting_type>(setting));
  if(!inserted)
  {
    THROW_EXCEPTION(util::exception(std::format("setting already created: path{}, name={}", value_path, value_name)));
  }
  return setting_ptr;
}

} // namespace bibstd::workflow
