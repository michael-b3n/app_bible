#pragma once

#include "bibstd/bible/parser.hpp"
#include "bibstd/bible/reference.hpp"
#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <memory>
#include <mutex>

namespace bibstd::core
{
// Forward declarations
class core_scripture_store;
} // namespace bibstd::core
namespace bibstd::workflow
{

///
/// Settings corresponding to workflow scripture.
///
class workflow_scripture_settings final : public framework::settings_base
{
public: // Structors
  workflow_scripture_settings();
  ~workflow_scripture_settings() noexcept = default;

public: // Variables
  const setting_type<std::optional<std::string>> scripture_name;
};

///
/// Workflow for scripture.
///
class workflow_scripture final
  : public workflow_base<workflow_scripture>
  , public framework::settings_owner<workflow_scripture_settings>
{
public: // Typedefs
  struct params final
  {
    bible::reference reference;
    std::optional<std::string> scripture_name;
  };

  using process_params = framework::process_params<params>;
  using process_result = framework::process_result<bible::parser::html_passage>;

public: // Structors
  workflow_scripture();
  ~workflow_scripture() noexcept;

public: // Modifiers
  ///
  /// Get scripture passage.
  /// \param params Process parameters containing the reference and optional scripture name
  /// \return scripture passage, or an unexpected result in case of failure
  ///
  auto get(const process_params& params) -> process_result;

private: // Implementation
  auto init() -> void;

private: // Variables
  mutable std::mutex mtx_;
  const std::unique_ptr<core::core_scripture_store> core_scripture_store_;
};

} // namespace bibstd::workflow
