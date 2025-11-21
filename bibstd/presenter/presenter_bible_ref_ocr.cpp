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
  , recognize_largest_bounding_box{workflow_settings_->create_setting("ocr.recognize_largest_bounding_box", false)}
// clang-format on
{
}

///
///
presenter_bible_ref_ocr::presenter_bible_ref_ocr()
  : workflow_bible_ref_ocr_{std::make_unique<workflow::workflow_bible_ref_ocr>()}
{
  // Connect workflow signals to presenter signals
  connections_.add_connection(workflow_bible_ref_ocr_->connect<workflow::workflow_bible_ref_ocr::signal_id::started>(
    [this]([[maybe_unused]] const auto& /*params*/) { emit<signal_id::started>(); }
  ));
  connections_.add_connection(workflow_bible_ref_ocr_->connect<workflow::workflow_bible_ref_ocr::signal_id::ended>(
    [this]([[maybe_unused]] const auto& /*params*/) { emit<signal_id::ended>(); }
  ));
}

///
///
presenter_bible_ref_ocr::~presenter_bible_ref_ocr() noexcept = default;

///
///
auto presenter_bible_ref_ocr::start(const util::screen_coordinates_type& position) -> std::stop_source
{
  emit<signal_id::queued>(position.x(), position.y());

  const auto start_params = workflow::workflow_bible_ref_ocr::start_params{
    {position, settings->recognize_largest_bounding_box->value()}
  };
  return workflow_bible_ref_ocr_->start(start_params);
}

} // namespace bibstd::presenter
