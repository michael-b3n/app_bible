#pragma once

#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/workflow/workflow_base.hpp"

namespace bibstd::workflow
{

///
/// Settings corresponding to workflow template.
///
class workflow_template_settings final : public framework::settings_base
{
public: // Structors
  workflow_template_settings();
  ~workflow_template_settings() noexcept = default;

public: // Variables
  const setting_type<std::optional<std::string>> text;
  const setting_type<bool> flag;
};

///
/// Workflow template. This is a template for creating new workflows.
///
class workflow_template final
  : public workflow_base<workflow_template>
  , public framework::settings_owner<workflow_template_settings>
{
public: // Typedefs
  struct params final
  {
    int value{0}; /*some type*/
  };
  using result = float /*some type*/;

  using process_params = framework::process_params<params>;
  using process_result = framework::process_result<result>;

public: // Structors
  workflow_template();
  ~workflow_template() noexcept;

public: // Modifiers
  ///
  /// Start the workflow template.
  /// \param params Process parameters for the workflow
  /// \return result, or an unexpected result in case of failure
  ///
  auto start(const process_params& params) -> process_result;
};

} // namespace bibstd::workflow
