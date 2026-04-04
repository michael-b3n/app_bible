#pragma once

#include <bibstd/util/scoped_guard.hpp>
#include <bibstd/workflow/workflow_base.hpp>

#include <memory>

namespace aba
{

///
/// Struct containing all backend components.
///
struct backend_instance final
{
  // Static modifiers
  bibstd::util::scoped_guard pool_guard;

  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_hotkey;
  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_scripture;
  std::shared_ptr<bibstd::workflow::workflow_ground> workflow_bible_ref_ocr;

  bibstd::util::scoped_guard hotkey_guard;
};

///
/// Initialize backend components.
/// \return backend instance
///
auto construct_backend() -> backend_instance;

} // namespace aba
