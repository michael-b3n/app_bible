#pragma once

#include <bibstd/util/scope_guard.hpp>
#include <bibstd/workflow/workflow_base.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <memory>

namespace aba
{

///
/// Struct containing all backend components.
///
struct backend_instance final
{
  std::shared_ptr<bibstd::workflow::workflow_settings> workflow_settings;
  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_hotkey;
  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_scripture;
  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_bible_ref_ocr;

  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_template;
};

///
/// Initialize backend components.
/// \return backend instance
///
auto construct_backend() -> backend_instance;

} // namespace aba
