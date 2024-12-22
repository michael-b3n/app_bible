#pragma once

#include "meta/for_each.hpp"
#include "meta/pack.hpp"
#include "system/filesystem.hpp"
#include "util/exception.hpp"
#include "util/non_owning_ptr.hpp"
#include "util/property_tree.hpp"

#include <filesystem>
#include <format>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

namespace bibstd::core
{

///
/// Core setting. This class owns setting objects.
/// \tparam ...Args List of all settings types.
///
template<typename... Args>
class core_settings final
{
public: // Typedefs
  template<typename T>
    requires meta::contains_v<meta::pack<Args...>, T>
  struct named_value final
  {
    using value_type = T;
    std::string category;
    std::string name;
    T value;
  };

  template<typename T>
  using property_non_owning_ptr_type = util::non_owning_ptr<util::property<named_value<T>>>;
  template<typename T>
  using property_uptr_type = std::unique_ptr<util::property<named_value<T>>>;

  using property_uptr_variant_type = meta::for_each_t<std::variant<Args...>, property_uptr_type>;
  using property_non_owning_ptr_variant_type = meta::for_each_t<std::variant<Args...>, property_non_owning_ptr_type>;

public: // Constants
  static constexpr std::string_view settings_file_name = "settings.xml";
  inline static const std::filesystem::path settings_file_path{system::filesystem::local_data_folder() / settings_file_name};

public: // Structors
  core_settings() = default;

public: // Accessors
  ///
  /// Access all created settings of this core.
  /// \return list of all created settings
  ///
  auto settings() -> std::vector<property_non_owning_ptr_variant_type>;

public: // Modifiers
  ///
  /// Create a new setting owned by this core. The setting type should be a named value.
  /// \param value_path Value path must be unique
  /// \param category Category of setting
  /// \param value_name Name of setting
  /// \param default_value Default value of setting
  /// \return the newly created setting accessed via a property
  ///
  template<typename T>
  [[nodiscard]] auto create_setting(
    const std::string& value_path, std::string&& category, std::string&& value_name, T&& default_value) -> property_non_owning_ptr_type<T>
    requires meta::contains_v<meta::pack<Args...>, T>;

private: // Variables
  std::map<std::string, property_uptr_variant_type> property_map_{};
  util::property_tree::sptr_type tree_{util::property_tree::create(settings_file_path)};
};

///
///
template<typename... Args>
auto core_settings<Args...>::settings() -> std::vector<property_non_owning_ptr_variant_type>
{
  auto retval = std::vector<property_non_owning_ptr_variant_type>(property_map_.size());
  const auto to_ptr = [](const auto& uptr)
  { return std::visit([](const auto& e) -> property_non_owning_ptr_variant_type { return e.get(); }, uptr); };
  for(const auto [i, ptr] : property_map_ | std::views::values | std::views::transform(to_ptr) | std::views::enumerate)
  {
    retval.at(i) = ptr;
  }
  return retval;
}

///
///
template<typename... Args>
template<typename T>
auto core_settings<Args...>::create_setting(
  const std::string& value_path, std::string&& category, std::string&& value_name, T&& default_value) -> property_non_owning_ptr_type<T>
  requires meta::contains_v<meta::pack<Args...>, T>
{
  auto prop_value = named_value<T>{.category = std::move(category), .name = std::move(value_name), .value = std::move(default_value)};
  auto prop = std::make_unique<util::property<named_value<T>>>(
    std::move(tree_->create_property(util::property_tree::path_type{value_path}, std::move(prop_value))));
  const auto prop_ptr = prop.get();
  const auto [_, inserted] = property_map_.emplace(value_path, std::move(prop));
  if(!inserted)
  {
    THROW_EXCEPTION(util::exception(std::format("Setting already created: path{}, name={}", value_path, value_name)));
  }
  return prop_ptr;
}

} // namespace bibstd::core
