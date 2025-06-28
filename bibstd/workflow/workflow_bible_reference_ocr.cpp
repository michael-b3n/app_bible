#include "workflow/workflow_bible_reference_ocr.hpp"
#include "bible/reference_range.hpp"
#include "core/core_bible_reference.hpp"
#include "core/core_bible_reference_ocr.hpp"
#include "core/core_bibleserver_lookup.hpp"
#include "core/core_settings.hpp"
#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "system/screen.hpp"
#include "util/format.hpp"

#include <array>
#include <numeric>
#include <thread>

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_bible_reference_ocr_settings::workflow_bible_reference_ocr_settings()
  : app_framework::settings_base{"OCR"}
  , translations{core_settings_->create_setting("ocr.translations", "Translations", std::vector<bible::translation>{bible::translation::ngu, bible::translation::elb})}
// clang-format on
{
}

///
///
workflow_bible_reference_ocr::workflow_bible_reference_ocr(language language)
  : core_bible_reference_ocr_{std::make_unique<core::core_bible_reference_ocr>(language)}
  , core_bible_reference_{std::make_unique<core::core_bible_reference>()}
  , core_bibleserver_lookup_{std::make_unique<core::core_bibleserver_lookup>()}
{
}

///
///
workflow_bible_reference_ocr::~workflow_bible_reference_ocr() noexcept = default;

///
///
auto workflow_bible_reference_ocr::find_references(const settings_type& settings) -> void
{
  const auto cursor_position = system::screen::cursor_position();
  app_framework::thread_pool::queue_task(
    [this, settings, cursor_position]()
    {
      settings_ = settings;
      LOG_INFO("find references: cursor_position={}", cursor_position);
      const auto references = find_references_impl(cursor_position);
      LOG_INFO("OCR reference search finished: references=[{}]", util::format::join(references, ", "));
      std::ranges::for_each(
        references,
        [&](const auto& reference_range) { core_bibleserver_lookup_->open(reference_range, settings_->translations->value()); }
      );
    },
    strand_id_
  );
}

///
///
auto workflow_bible_reference_ocr::find_references_impl(const screen_coordinates_type& cursor_position)
  -> std::vector<bible::reference_range>
{
  auto result = std::vector<bible::reference_range>{};
  const auto relative_cursor_pos = core_bible_reference_ocr_->capture_ocr_image(cursor_position);
  if(!relative_cursor_pos)
  {
    LOG_WARN("capture screen failed: cursor_position={}", cursor_position);
    return result;
  }
  const auto bounding_boxes = core_bible_reference_ocr_->recognize_bounding_box(*relative_cursor_pos);
  if(!bounding_boxes)
  {
    LOG_DEBUG("missing bounding box: relative_cursor_position={}", *relative_cursor_pos);
    return result;
  }

  std::ignore = std::ranges::any_of(
    std::views::iota(std::size_t{0}, core::core_bible_reference_ocr::recognition_area_step_count),
    [&](const auto step_index)
    {
      if(!core_bible_reference_ocr_->recognize_capture_area(*bounding_boxes, step_index))
      {
        LOG_DEBUG(
          "recognize capture area failed: bounding_box={}, step_index={}", bounding_boxes->initial, step_index
        );
        return false;
      }
      auto references = parse_tesseract_recognition(*relative_cursor_pos);
      if(!references.empty())
      {
        result = std::move(references);
        return true;
      }
      LOG_DEBUG(
        "parse recognition result: references=[{}], relative_cursor_pos={}, step_index={}",
        util::format::join(references, ", "),
        *relative_cursor_pos,
        step_index
      );
      return false;
    }
  );
  return result;
}

///
///
auto workflow_bible_reference_ocr::parse_tesseract_recognition(const screen_coordinates_type& relative_cursor_pos)
  -> std::vector<bible::reference_range>
{
  auto references = std::vector<bible::reference_range>{};
  const auto position_data = core_bible_reference_ocr_->find_main_reference_position_data(relative_cursor_pos);
  if(position_data)
  {
    auto parse_result = core_bible_reference_->parse(position_data->text, position_data->cursor_character_index);
    // If no references are found, we parse other high confidence OCR choices.
    // If a parse result is found and the area is valid we break out.
    if(parse_result.ranges.empty())
    {
      const auto position_data_choices =
        core_bible_reference_ocr_->find_reference_position_data_from_choices(relative_cursor_pos);
      std::ranges::any_of(
        position_data_choices,
        [&](const auto& position_data_choice)
        {
          parse_result = core_bible_reference_->parse(position_data_choice.text, position_data_choice.cursor_character_index);
          return !parse_result.ranges.empty();
        }
      );
    }
    references = std::move(parse_result.ranges);
  }
  return references;
}

} // namespace bibstd::workflow
