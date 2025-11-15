#include "bibstd/presenter/presenter_bible_ref_ocr.hpp"
#include "bibstd/system/hotkey.hpp"
#include "bibstd/system/screen.hpp"
#include "bibstd/workflow/workflow_bible_ref_ocr.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

namespace bibstd::presenter
{

///
///
// clang-format off
presenter_bible_ref_ocr_settings::presenter_bible_ref_ocr_settings()
  : hotkey_modifier{workflow_settings_->create_setting("ocr.hotkey_modifier", system::hotkey_common::key_modifier::alt)}
  , hotkey{workflow_settings_->create_setting("ocr.hotkey", system::hotkey_common::key::vk_f)}
// clang-format on
{
}

///
///
presenter_bible_ref_ocr::presenter_bible_ref_ocr()
  : workflow_bible_ref_ocr_{std::make_unique<workflow::workflow_bible_ref_ocr>()}
{
  // Connect signals
  connections_.add_connection(workflow_bible_ref_ocr_->connect<signal::common_id::started>([this](...) {}));
  connections_.add_connection(workflow_bible_ref_ocr_->connect<signal::common_id::ended>([this](...) {}));

  // Register hotkeys. Currently no hotkey change is supported.
  system::hotkey::register_callback(
    settings->hotkey->value(),
    settings->hotkey_modifier->value(),
    [this, stop_source = std::stop_source()]() mutable
    {
      stop_source.request_stop();
      const auto start_params = workflow::workflow_bible_ref_ocr::start_params{
        {system::screen::cursor_position(), false}
      };
      stop_source = workflow_bible_ref_ocr_->start(start_params);
    }
  );
}

///
///
presenter_bible_ref_ocr::~presenter_bible_ref_ocr() noexcept = default;

} // namespace bibstd::presenter
