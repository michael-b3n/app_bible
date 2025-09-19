#pragma once

#include "workflow/workflow_settings_common.hpp"

namespace bibstd::workflow
{
// Forward declarations
class workflow_settings;
} // namespace bibstd::workflow

namespace bibstd::framework
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
  using setting_type = workflow::workflow_settings_common::setting_non_owning_ptr_type<T>;

protected: // Variables
  std::unique_ptr<workflow::workflow_settings> workflow_settings_;
};

} // namespace bibstd::framework
