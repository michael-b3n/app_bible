#include "bibstd/presenter/presenter_bible_ref_ocr.hpp"
#include "bibstd/workflow/workflow_bible_ref_lookup.hpp"
#include "bibstd/workflow/workflow_bible_ref_ocr.hpp"
#include "bibstd/workflow/workflow_scripture.hpp"
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
  : workflow_bible_ref_lookup_{std::make_unique<workflow::workflow_bible_ref_lookup>()}
  , workflow_bible_ref_ocr_{std::make_unique<workflow::workflow_bible_ref_ocr>()}
  , workflow_scripture_{std::make_unique<workflow::workflow_scripture>()}
{
  connections_.add_connection(workflow_bible_ref_ocr_->connect<workflow::workflow_bible_ref_ocr::signal_id::ended>(
    [this](const auto& result)
    {
      if(result.result)
      {
        workflow_bible_ref_lookup_->lookup({{*result.result}});
      }
      emit<signal_id::found>(result.process_id, result.result.value_or({}));
    }
  ));
  connections_.add_connection(workflow_bible_ref_lookup_->connect<workflow::workflow_bible_ref_lookup::signal_id::ended>(
    [this](const auto& result) { emit<signal_id::lookup_ended>(result.process_id); }
  ));
}

///
///
presenter_bible_ref_ocr::~presenter_bible_ref_ocr() noexcept = default;

///
///
auto presenter_bible_ref_ocr::start(const util::screen_coordinates_type& position) -> std::stop_source
{
  const auto start_params = workflow::workflow_bible_ref_ocr::start_params{{position}};
  emit<signal_id::started>(start_params.process_id());
  return workflow_bible_ref_ocr_->start(start_params);
}

} // namespace bibstd::presenter
