#include "workflow/workflow_bible_reference_ocr.hpp"
#include "core/core_bible_reference.hpp"
#include "core/core_bibleserver_lookup.hpp"
#include "core/core_settings.hpp"
#include "core/core_tesseract.hpp"
#include "data/save_as_bitmap.hpp"
#include "system/filesystem.hpp"
#include "system/screen.hpp"
#include "txt/chars.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/date.hpp"

#include <array>
#include <numeric>
#include <thread>

namespace bibstd::workflow
{

///
///
workflow_bible_reference_ocr_settings::workflow_bible_reference_ocr_settings()
  : app_framework::settings_base{"OCR"} // clang-format off
  , cursor_movement_cooldown{core_settings_->create_setting("ocr.cursor_movement_cooldown", "Cursor Movement Cooldown", std::chrono::milliseconds{500})}
  , translations{core_settings_->create_setting("ocr.translations", "Translations", std::vector<bible::translation>{bible::translation::ngu, bible::translation::elb})}
// clang-format on
{
}

///
///
workflow_bible_reference_ocr::workflow_bible_reference_ocr(language language)
  : core_bible_reference_{std::make_unique<core::core_bible_reference>()}
  , core_bibleserver_lookup_{std::make_unique<core::core_bibleserver_lookup>()}
  , core_tesseract_{std::make_unique<core::core_tesseract>(language)}
{
}

///
///
workflow_bible_reference_ocr::~workflow_bible_reference_ocr() noexcept = default;

///
///
auto workflow_bible_reference_ocr::run_once(const settings_type& settings) -> void
{
  app_framework::thread_pool::queue_task(
    [this, settings]()
    {
      settings_ = settings;
      task();
    },
    strand_id_
  );
}

///
///
auto workflow_bible_reference_ocr::task() -> void
{
  data_.current_cursor_position = system::screen::cursor_position();
  const auto window_rect = system::screen::window_at(data_.current_cursor_position);
  if(!window_rect)
  {
    LOG_WARN(
      log_channel, "No window at cursor position=[{}, {}]", data_.current_cursor_position.x(), data_.current_cursor_position.y()
    );
    return;
  }

  data_.current_img_area = *window_rect;
  auto success = false;
  const auto changed = core_tesseract_->set_image(
    [this, &success](auto& pixel_plane)
    {
      success = capture_img(pixel_plane);
      if(!success)
      {
        LOG_WARN(log_channel, "Capture screen failed: area_width={}, area_height={}", pixel_plane.width, pixel_plane.height);
      }
    }
  );
  if(success)
  {
    if(changed)
    {
      core_tesseract_->recognize(std::nullopt);
    }
    const auto relative_cursor_position = data_.current_cursor_position - data_.current_img_area.origin();
    core_tesseract_->for_each_until(
      core::core_tesseract::text_resolution::paragraph,
      [&](const auto text_paragraph, const auto& bounding_box)
      {
        using rect_type = std::decay_t<decltype(bounding_box)>;
        const auto found = rect_type::contains(bounding_box, relative_cursor_position);
        if(found)
        {
          auto text = std::string{};
          text.reserve(text_paragraph.size());
          std::vector<double> distance_to_cursor{};
          distance_to_cursor.reserve(text_paragraph.size());

          core_tesseract_->for_each(
            core::core_tesseract::text_resolution::character,
            [&](const auto text_character, const auto& bounding_box)
            {
              text.append(text_character.data(), text_character.size());
              const auto abs_distance =
                std::abs(screen_coordinates_type::distance(bounding_box.center(), relative_cursor_position));
              distance_to_cursor.insert(distance_to_cursor.cend(), text_character.size(), abs_distance);
            }
          );
          if(const auto min_element =
               std::ranges::min_element(distance_to_cursor, [](const auto& a, const auto& b) { return a < b; });
             min_element != std::ranges::cend(distance_to_cursor))
          {
            const auto reference_ranges =
              core_bible_reference_->parse(text, std::ranges::distance(distance_to_cursor.cbegin(), min_element));

            std::ranges::for_each(
              reference_ranges,
              [&](const auto& reference_range)
              { core_bibleserver_lookup_->open(reference_range, settings_->translations->value()); }
            );
          }
        }
        return found;
      }
    );
  }
}

///
///
auto workflow_bible_reference_ocr::capture_img(data::plane<data::pixel>& pixel_plane) -> bool
{
  pixel_plane.width = data_.current_img_area.horizontal_range();
  pixel_plane.height = data_.current_img_area.vertical_range();
  if(pixel_plane.data.size() < pixel_plane.width * pixel_plane.height)
  {
    pixel_plane.data.resize(pixel_plane.width * pixel_plane.height);
  }
  return system::screen::capture(data_.current_img_area, pixel_plane);
}

} // namespace bibstd::workflow
