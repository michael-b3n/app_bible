#include "src/construct_backend.hpp"

#include <bibstd/framework/thread_pool.hpp>
#include <bibstd/system/hotkey.hpp>

#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <memory>

namespace aba
{

///
///
auto construct_backend() -> backend_instance
{
  // Init backend
  // clang-format off
  auto workflow_settings = std::make_shared<bibstd::workflow::workflow_settings>();
  auto workflow_hotkey = std::make_shared<bibstd::workflow::workflow_hotkey>();
  auto workflow_scripture = std::make_shared<bibstd::workflow::workflow_scripture>(workflow_settings);
  auto workflow_bible_ref_ocr = std::make_shared<bibstd::workflow::workflow_bible_ref_ocr>(workflow_settings, workflow_scripture);
  // clang-format on

  // Construct workflows here. The returned scoped guard will deinitialize the workflows when it goes out of scope.
  return backend_instance{
    .workflow_settings{std::move(workflow_settings)},
    .workflow_hotkey{std::move(workflow_hotkey)},
    .workflow_scripture{std::move(workflow_scripture)},
    .workflow_bible_ref_ocr{std::move(workflow_bible_ref_ocr)}
  };
}

} // namespace aba
