#pragma once

#include "core/core_settings_common.hpp"

namespace bibstd::core
{
// Forward declarations
class core_settings;
} // namespace bibstd::core

namespace bibstd::app_framework
{

///
/// Settings base class.
///
class settings_base
{
public: // Structors
  settings_base(std::string&& parent);
  virtual ~settings_base() noexcept;

protected: // Typedefs
  template<typename T>
  using setting_type = core::core_settings_common::setting_non_owning_ptr_type<T>;

protected: // Variables
  std::unique_ptr<core::core_settings> core_settings_;
};

} // namespace bibstd::app_framework
