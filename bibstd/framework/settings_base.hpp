#pragma once

#include "bibstd/workflow/workflow_settings.hpp"

#include <memory>
#include <type_traits>

namespace bibstd::framework
{

///
/// Settings base class.
///
class settings_base
{
public: // Structors
  settings_base(std::shared_ptr<workflow::workflow_settings> workflow_settings);
  virtual ~settings_base() noexcept;
  settings_base(const settings_base&) = delete;
  settings_base(settings_base&&) = delete;
  auto operator=(const settings_base&) -> settings_base& = delete;
  auto operator=(settings_base&&) -> settings_base& = delete;

protected: // Typedefs
  template<typename T>
  using setting_value_t = typename std::remove_cv_t<std::remove_pointer_t<T>>::value_type;

  template<typename T>
  using setting_type = workflow::workflow_settings::setting_non_owning_ptr_type<T>;

protected: // Variables
  std::shared_ptr<workflow::workflow_settings> workflow_settings_;
};

} // namespace bibstd::framework
