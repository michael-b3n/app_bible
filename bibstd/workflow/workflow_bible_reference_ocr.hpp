#pragma once

#include "app_framework/settings_base.hpp"
#include "app_framework/thread_pool.hpp"
#include "core/core_tesseract_common.hpp"
#include "data/pixel.hpp"
#include "data/plane.hpp"
#include "system/screen.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string_view>

namespace bibstd::core
{
// Forward declarations
class core_bible_reference;
class core_bibleserver_lookup;
class core_tesseract;
} // namespace bibstd::core

namespace bibstd::workflow
{

///
/// Workflow bible reference ocr.
///
class workflow_bible_reference_ocr_settings final : public app_framework::settings_base
{
public: // Typedefs
  using sptr_type = std::shared_ptr<workflow_bible_reference_ocr_settings>;

public: // Structors
  workflow_bible_reference_ocr_settings();
  ~workflow_bible_reference_ocr_settings() noexcept = default;

public: // Variables
  const setting_type<std::chrono::milliseconds> cursor_movement_cooldown;
  const setting_type<std::vector<bible::translation>> translations;
};

///
/// Workflow bible reference ocr.
///
class workflow_bible_reference_ocr final
{
public: // Constants
  static constexpr std::string_view log_channel = "workflow_bible_reference_ocr";

public: // Typedefs
  using settings_type = workflow_bible_reference_ocr_settings::sptr_type;
  using language = core::core_tesseract_common::language;

public: // Structors
  workflow_bible_reference_ocr(language language);
  ~workflow_bible_reference_ocr() noexcept;

public: // Modifiers
  auto run_once(const settings_type& settings) -> void;

private: // Typedefs
  using screen_rect_type = system::screen::screen_rect_type;
  using screen_coordinates_type = system::screen::screen_coordinates_type;

  ///
  /// Internal data structure.
  ///
  struct data_t final
  {
    std::chrono::time_point<std::chrono::system_clock> last_cursor_movement{std::chrono::system_clock::now()};
    screen_coordinates_type current_cursor_position{0, 0};
    screen_rect_type current_img_area{
      {0, 0},
      0, 0
    };
  };

private: // Implementation
  auto task() -> void;
  auto capture_img(data::plane<data::pixel>& pixel_plane) -> bool;

private: // Variables
  const std::unique_ptr<core::core_bible_reference> core_bible_reference_;
  const std::unique_ptr<core::core_bibleserver_lookup> core_bibleserver_lookup_;
  const std::unique_ptr<core::core_tesseract> core_tesseract_;

  const app_framework::thread_pool::strand_id_type strand_id_{app_framework::thread_pool::strand_id()};
  settings_type settings_{nullptr};
  data_t data_;
};

} // namespace bibstd::workflow
