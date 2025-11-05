#pragma once

#include "workflow/workflow_settings.hpp"

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
  using setting_type = workflow::workflow_settings::setting_non_owning_ptr_type<T>;

protected: // Variables
  std::unique_ptr<workflow::workflow_settings> workflow_settings_;
};

} // namespace bibstd::framework
