#pragma once

#include "bible/common.hpp"
#include "meta/contains.hpp"
#include "meta/for_each.hpp"
#include "system/filesystem.hpp"
#include "util/non_owning_ptr.hpp"
#include "util/property.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace bibstd::core
{

///
/// Core setting common. This helper struct contains helper types for core settings.
///
struct core_settings_common final
{
  // Typedefs
  using setting_types = std::variant<
    // Basic types
    bool,
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t,
    std::uint8_t,
    std::uint16_t,
    std::uint32_t,
    std::uint64_t,
    float,
    double,
    std::string,
    std::chrono::nanoseconds,
    std::chrono::milliseconds,
    std::chrono::seconds,
    std::vector<std::int32_t>,
    std::vector<std::int64_t>,
    std::vector<std::uint32_t>,
    std::vector<std::uint64_t>,
    std::vector<double>,
    std::vector<std::string>,

    // Bible types
    bible::book_id,
    std::vector<bible::book_id>,
    bible::testament_id,
    std::vector<bible::testament_id>,
    bible::translation,
    std::vector<bible::translation>>;

  ///
  /// Setting class.
  ///
  template<typename T>
    requires meta::contains_v<setting_types, T>
  class setting final
  {
  public: // Typedefs
    using value_type = T;

  public: // Structors
    setting(const std::string& parent, const std::string& name, util::property<T>&& value);

  public: // Accessors
    ///
    /// Access setting value.
    /// \return reference to setting value
    ///
    auto value() const -> T;

  public: // Setters
    ///
    /// Set setting value.
    /// \param v setting value that shall be set
    ///
    auto value(const T& v) -> void;

  public: // Variables
    const std::string parent;
    const std::string name;

  private: // Variables
    util::property<T> value_;
  };

  template<typename T>
  using setting_non_owning_ptr_type = util::non_owning_ptr<setting<T>>;
  template<typename T>
  using setting_uptr_type = std::unique_ptr<setting<T>>;

  using setting_uptr_variant_type = meta::for_each_t<setting_types, setting_uptr_type>;
  using setting_non_owning_ptr_variant_type = meta::for_each_t<setting_types, setting_non_owning_ptr_type>;

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
template<typename T>
  requires meta::contains_v<core_settings_common::setting_types, T>
core_settings_common::setting<T>::setting(const std::string& parent_, const std::string& name_, util::property<T>&& value)
  : parent{parent_}
  , name{name_}
  , value_{std::move(value)}
{
}

///
///
template<typename T>
  requires meta::contains_v<core_settings_common::setting_types, T>
auto core_settings_common::setting<T>::value() const -> T
{
  return value_.value();
}

///
///
template<typename T>
  requires meta::contains_v<core_settings_common::setting_types, T>
auto core_settings_common::setting<T>::value(const T& v) -> void
{
  value_.value(v);
}

///
///
auto core_settings_common::settings_file_path() -> const std::filesystem::path&
{
  static const std::filesystem::path path{system::filesystem::local_data_folder() / settings_file_name};
  return path;
}

} // namespace bibstd::core
