#include "bibstd/workflow/workflow_bible_ref_ocr.hpp"
#include "bibstd/bible/reference_range.hpp"
#include "bibstd/core/core_bible_ref.hpp"
#include "bibstd/core/core_bible_ref_ocr.hpp"
#include "bibstd/core/core_bibleserver_lookup.hpp"
#include "bibstd/util/format.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_bible_ref_ocr_settings::workflow_bible_ref_ocr_settings()
  : tessdata_path{workflow_settings_->create_setting("ocr.tessdata_path", core::core_tesseract_common::tessdata_folder_finder())}
  , language{workflow_settings_->create_setting("ocr.language", core::core_tesseract_common::language::de)}
  , translations{workflow_settings_->create_setting("ocr.translations", std::vector<bible::translation>{bible::translation::ngu, bible::translation::elb})}
  , recognize_largest_bounding_box{workflow_settings_->create_setting("ocr.recognize_largest_bounding_box", false)}
// clang-format on
{
}

///
///
workflow_bible_ref_ocr::workflow_bible_ref_ocr()
  : core_bible_ref_{std::make_unique<core::core_bible_ref>()}
  , core_bibleserver_lookup_{std::make_unique<core::core_bibleserver_lookup>()}
{
  if(!settings->tessdata_path->value() || !std::filesystem::exists(*settings->tessdata_path->value()))
  {
    if(const auto found_tessdata_folder = core::core_tesseract_common::tessdata_folder_finder())
    {
      LOG_WARN("tessdata path setting invalid: used_alternative_folder=\"{}\"", found_tessdata_folder->generic_string());
      settings->tessdata_path->value(*found_tessdata_folder);
    }
  }
  const auto path = settings->tessdata_path->value();
  if(path && std::filesystem::exists(*path))
  {
    core_bible_ref_ocr_ = std::make_shared<core::core_bible_ref_ocr>(*path, settings->language->value());
  }
}

///
///
workflow_bible_ref_ocr::~workflow_bible_ref_ocr() noexcept = default;

///
///
auto workflow_bible_ref_ocr::start(const start_params& params) -> std::stop_source
{
  const std::stop_source stop_source;
  try
  {
    const auto core_bible_ref_ocr = core_bible_ref_ocr_.load();
    if(!core_bible_ref_ocr)
    {
      LOG_WARN("failed to start bible reference ocr search: not fully initialized");
      emit<signal_id::ended>(result_type{params.process_id(), return_failure});
      return {};
    }
    // Capture screen directly on call of this function to ensure the cursor position is up-to-date.
    // This is usually called from the main thread and takes only a few milliseconds.
    // This also ensures that no displayed windows are blocking the screen capture.
    auto image_data = core_bible_ref_ocr->capture_screen(params->cursor_position);
    if(!image_data)
    {
      LOG_WARN("capture screen failed: cursor_position={}", params->cursor_position);
      emit<signal_id::ended>(result_type{params.process_id(), return_failure});
      return {};
    }
    LOG_INFO("find references: cursor_position={}", params->cursor_position);
    const std::stop_source stop_source;
    framework::thread_pool::queue_task(
      [this, params, core_bible_ref_ocr, data = std::move(*image_data), token = stop_source.get_token()]() mutable
      {
        try
        {
          const auto translations = settings->translations->value();
          const auto references =
            find_references(core_bible_ref_ocr, std::move(data), settings->recognize_largest_bounding_box->value(), token);
          if(references.has_value())
          {
            LOG_INFO("reference search finished: references=[{}]", util::format::join(*references, ", "));
            std::ranges::for_each(
              *references, [&](const auto& reference_range) { core_bibleserver_lookup_->open(reference_range, translations); }
            );
          }
          emit<signal_id::ended>(result_type{params.process_id(), references});
        }
        catch(const util::exception& e)
        {
          LOG_ERROR("exception occurred: {}", e);
          emit<signal_id::ended>(result_type{params.process_id(), return_failure});
        }
      },
      strand_id_
    );
  }
  catch(const util::exception& e)
  {
    LOG_ERROR("exception occurred: {}", e);
    emit<signal_id::ended>(result_type{params.process_id(), return_failure});
  }
  return stop_source;
}

///
///
auto workflow_bible_ref_ocr::find_references(
  const std::shared_ptr<core::core_bible_ref_ocr>& core_bible_ref_ocr,
  auto&& image_data,
  const bool recognize_largest_bounding_box,
  const std::stop_token stop_token
) -> decltype(result_type::result)
{
  if(stop_token.stop_requested())
  {
    return return_stopped;
  }
  const auto relative_cursor_pos = image_data.relative_cursor_pos;
  const auto bounding_boxes = core_bible_ref_ocr->recognize_bounding_box(std::move(image_data));
  if(!bounding_boxes)
  {
    LOG_DEBUG("missing bounding box: relative_cursor_pos={}", relative_cursor_pos);
    return {};
  }
  if(stop_token.stop_requested())
  {
    return return_stopped;
  }
  if(!core_bible_ref_ocr->recognize_capture_area(*bounding_boxes, recognize_largest_bounding_box))
  {
    LOG_WARN(
      "recognize capture area failed: bounding_box={}",
      recognize_largest_bounding_box ? bounding_boxes->largest : bounding_boxes->reduced
    );
    return return_failure;
  }
  return parse_tesseract_recognition(core_bible_ref_ocr, relative_cursor_pos);
}

///
///
auto workflow_bible_ref_ocr::parse_tesseract_recognition(
  const std::shared_ptr<core::core_bible_ref_ocr>& core_bible_ref_ocr, const util::screen_coordinates_type& relative_cursor_pos
) -> std::vector<bible::reference_range>
{
  auto references = std::vector<bible::reference_range>{};
  const auto position_data = core_bible_ref_ocr->find_main_reference_position_data(relative_cursor_pos);
  if(position_data)
  {
    auto parse_result = core_bible_ref_->parse(position_data->text, position_data->cursor_character_index);
    // If no references are found, we parse other high confidence OCR choices.
    // If a parse result is found and the area is valid we break out.
    if(parse_result.ranges.empty())
    {
      const auto position_data_choices = core_bible_ref_ocr->find_reference_position_data_from_choices(relative_cursor_pos);
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
