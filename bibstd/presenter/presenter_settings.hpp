#pragma once

#include "bibstd/workflow/workflow_settings.hpp"

#include <optional>
#include <vector>

namespace bibstd::presenter
{

///
/// Presenter settings.
/// Forwards access to the static workflow settings interface.
///
class presenter_settings final
{
public: // Typedefs
  using setting_type_erased_non_owning_ptr_variant_type =
    workflow::workflow_settings::setting_type_erased_non_owning_ptr_variant_type;

public: // Structors
  presenter_settings() = delete;

public: // Static interface
  ///
  /// Access all created settings.
  /// \return list of all created settings
  ///
  static auto type_erased_settings() -> std::vector<setting_type_erased_non_owning_ptr_variant_type>;

  ///
  /// Access a type erased setting for the specified path.
  /// \param path Setting path
  /// \return type erased setting or nullopt if no setting with the specified path exists
  ///
  static auto type_erased_setting(const std::string& path) -> std::optional<setting_type_erased_non_owning_ptr_variant_type>;
};

} // namespace bibstd::presenter
