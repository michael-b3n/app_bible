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
  , assumed_initial_char_height{core_settings_->create_setting("ocr.assumed_initial_char_height", "Assumed Initial Char Height", std::uint16_t{40})}
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
auto workflow_bible_reference_ocr::run_once(const settings_type& settings) -> void
{
  const auto cursor_position = system::screen::cursor_position();
  app_framework::thread_pool::queue_task(
    [this, settings, cursor_position]()
    {
      settings_ = settings;
      find_references(cursor_position);
    },
    strand_id_,
    app_framework::thread_pool::queue_rule::overwrite
  );
}

///
///
auto workflow_bible_reference_ocr::find_references(const screen_coordinates_type& cursor_pos) -> void
{
  const auto capture_areas =
    core_bible_reference_ocr_->generate_capture_areas(cursor_pos, settings_->assumed_initial_char_height->value());
  if(capture_areas.empty())
  {
    LOG_WARN("failed to define capture areas: cursor_position={}", cursor_pos);
    return;
  }

  std::vector<bible::reference_range> references{};
  std::ranges::any_of(
    capture_areas,
    [&](const auto& screen_area)
    {
      if(!core_bible_reference_ocr_->capture_and_recognize_area(screen_area))
      {
        LOG_WARN("capture screen failed: screen_area={}", screen_area);
        return false;
      }

      const auto image_dimensions = screen_rect_type({0, 0}, screen_area.horizontal_range(), screen_area.vertical_range());
      const auto relative_cursor_pos = cursor_pos - screen_area.origin();

      LOG_DEBUG(
        "start OCR: screen_area={}, cursor_position={}, image_dimensions={}, relative_cursor_pos={}",
        screen_area,
        cursor_pos,
        image_dimensions,
        relative_cursor_pos
      );

      const auto [valid_capture_area, found_references] = parse_tesseract_recognition(image_dimensions, relative_cursor_pos);
      if(!found_references.empty())
      {
        references = found_references;
      }
      return valid_capture_area;
    }
  );
  LOG_INFO("OCR reference search finished: found=[{}]", util::format::join(references, ", "));
  std::ranges::for_each(
    references,
    [&](const auto& reference_range) { core_bibleserver_lookup_->open(reference_range, settings_->translations->value()); }
  );
}

///
///
auto workflow_bible_reference_ocr::parse_tesseract_recognition(
  const screen_rect_type& image_dimensions, const screen_coordinates_type& relative_cursor_pos
) -> std::pair<bool, std::vector<bible::reference_range>>
{
  const auto paragraph_bounding_box_opt = core_bible_reference_ocr_->find_paragraph_bounding_box(relative_cursor_pos);
  if(!paragraph_bounding_box_opt)
  {
    return std::pair{false, std::vector<bible::reference_range>{}};
  }
  auto is_valid_capture_area = false;
  auto references = std::vector<bible::reference_range>{};
  const auto& paragraph_bounding_box = *paragraph_bounding_box_opt;
  const auto position_data = core_bible_reference_ocr_->find_main_reference_position_data(relative_cursor_pos);
  if(position_data)
  {
    auto parse_result = core_bible_reference_->parse(position_data->text, position_data->cursor_character_index);
    // Parse result might be empty but the capture area is still valid.
    // This is the case when the text is not a valid reference but the characters found in the image are
    // within the capture area including a safety margin. In this case no larger area is captured.
    is_valid_capture_area = core_bible_reference_ocr_->is_valid_capture_area(
      relative_cursor_pos, image_dimensions, paragraph_bounding_box, *position_data, parse_result.index_range_origin
    );
    // If the capture area is valid but no references are found, we parse other high confidence OCR choices.
    // If a parse result is found and the area is valid we break out.
    if(parse_result.ranges.empty() && is_valid_capture_area)
    {
      const auto position_data_choices =
        core_bible_reference_ocr_->find_reference_position_data_from_choices(relative_cursor_pos);
      std::ranges::any_of(
        position_data_choices,
        [&](const auto& position_data_choice)
        {
          parse_result = core_bible_reference_->parse(position_data_choice.text, position_data_choice.cursor_character_index);
          is_valid_capture_area = core_bible_reference_ocr_->is_valid_capture_area(
            relative_cursor_pos, image_dimensions, paragraph_bounding_box, position_data_choice, parse_result.index_range_origin
          );
          return is_valid_capture_area && !parse_result.ranges.empty();
        }
      );
      references = parse_result.ranges;
    }
    // If some references are found but the valid capture area is not valid, we keep the reference,
    // in case the OCR with larger images fail.
    // TODO: We want to return the found references after a certain timeout.
    references = parse_result.ranges;
  }
  LOG_DEBUG(
    "parse recognition result: references=[{}], valid_capture_area={}, image_dimensions={}, relative_cursor_pos={}",
    util::format::join(references, ", "),
    is_valid_capture_area,
    image_dimensions,
    relative_cursor_pos
  );
  return std::pair{is_valid_capture_area, references};
}

} // namespace bibstd::workflow
