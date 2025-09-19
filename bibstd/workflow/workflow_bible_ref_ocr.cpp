#include "workflow/workflow_bible_ref_ocr.hpp"
#include "bible/reference_range.hpp"
#include "core/core_bible_ref.hpp"
#include "core/core_bible_ref_ocr.hpp"
#include "core/core_bibleserver_lookup.hpp"
#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "system/screen.hpp"
#include "util/format.hpp"
#include "workflow/workflow_settings.hpp"

#include <array>
#include <numeric>
#include <thread>

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_bible_ref_ocr_settings::workflow_bible_ref_ocr_settings()
  : framework::settings_base{"OCR"}
  , language{workflow_settings_->create_setting("ocr.language", "Language", core::core_tesseract_common::language::de)}
  , translations{workflow_settings_->create_setting("ocr.translations", "Translations", std::vector<bible::translation>{bible::translation::ngu, bible::translation::elb})}
// clang-format on
{
}

///
///
workflow_bible_ref_ocr::workflow_bible_ref_ocr()
  : core_bible_ref_ocr_{std::make_unique<core::core_bible_ref_ocr>(settings->language->value())}
  , core_bible_ref_{std::make_unique<core::core_bible_ref>()}
  , core_bibleserver_lookup_{std::make_unique<core::core_bibleserver_lookup>()}
{
}

///
///
workflow_bible_ref_ocr::~workflow_bible_ref_ocr() noexcept = default;

///
///
auto workflow_bible_ref_ocr::start(const start_params& params) -> std::stop_source
{
  // Capture screen directly on call of this function to ensure the cursor position is up-to-date.
  // This is usually called from the main thread and takes only a few milliseconds.
  // This also ensures that no displayed windows are blocking the screen capture.
  auto image_data = core_bible_ref_ocr_->capture_screen(params->cursor_position);
  if(!image_data)
  {
    LOG_WARN("capture screen failed: cursor_position={}", params->cursor_position);
    return {};
  }
  LOG_INFO("find references: cursor_position={}", params->cursor_position);
  const std::stop_source stop_source;
  framework::thread_pool::queue_task(
    [this, params, data = std::move(*image_data), token = stop_source.get_token()]() mutable
    {
      emit<signal_id::started>(params);
      const auto translations = settings->translations->value();
      const auto references = find_references_impl(std::move(data), params->recognize_largest_bounding_box, token);
      if(references.has_value())
      {
        LOG_INFO("reference search finished: references=[{}]", util::format::join(*references, ", "));
        std::ranges::for_each(
          *references, [&](const auto& reference_range) { core_bibleserver_lookup_->open(reference_range, translations); }
        );
      }
      emit<signal_id::ended>(result_params{params.process_id(), references});
    },
    strand_id_
  );
  return stop_source;
}

///
///
auto workflow_bible_ref_ocr::find_references_impl(
  auto&& image_data, const bool recognize_largest_bounding_box, const std::stop_token stop_token
) -> result_type
{
  if(stop_token.stop_requested())
  {
    return return_stopped;
  }
  const auto relative_cursor_pos = image_data.relative_cursor_pos;
  const auto bounding_boxes = core_bible_ref_ocr_->recognize_bounding_box(std::move(image_data));
  if(!bounding_boxes)
  {
    LOG_DEBUG("missing bounding box: relative_cursor_pos={}", relative_cursor_pos);
    return {};
  }
  if(stop_token.stop_requested())
  {
    return return_stopped;
  }
  if(!core_bible_ref_ocr_->recognize_capture_area(*bounding_boxes, recognize_largest_bounding_box))
  {
    LOG_WARN(
      "recognize capture area failed: bounding_box={}",
      recognize_largest_bounding_box ? bounding_boxes->largest : bounding_boxes->reduced
    );
    return return_failure;
  }
  return parse_tesseract_recognition(relative_cursor_pos);
}

///
///
auto workflow_bible_ref_ocr::parse_tesseract_recognition(const util::screen_types::screen_coordinates_type& relative_cursor_pos)
  -> std::vector<bible::reference_range>
{
  auto references = std::vector<bible::reference_range>{};
  const auto position_data = core_bible_ref_ocr_->find_main_reference_position_data(relative_cursor_pos);
  if(position_data)
  {
    auto parse_result = core_bible_ref_->parse(position_data->text, position_data->cursor_character_index);
    // If no references are found, we parse other high confidence OCR choices.
    // If a parse result is found and the area is valid we break out.
    if(parse_result.ranges.empty())
    {
      const auto position_data_choices = core_bible_ref_ocr_->find_reference_position_data_from_choices(relative_cursor_pos);
      std::ranges::any_of(
        position_data_choices,
        [&](const auto& position_data_choice)
        {
          parse_result = core_bible_ref_->parse(position_data_choice.text, position_data_choice.cursor_character_index);
          return !parse_result.ranges.empty();
        }
      );
    }
    references = std::move(parse_result.ranges);
  }
  return references;
}

} // namespace bibstd::workflow
