#pragma once

#include "app_framework/settings_base.hpp"
#include "app_framework/thread_pool.hpp"
#include "bible/reference_range.hpp"
#include "core/core_bible_reference_ocr_common.hpp"
#include "core/core_tesseract_common.hpp"
#include "math/value_range.hpp"
#include "util/screen_types.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace bibstd::core
{
// Forward declarations
class core_bible_reference_ocr;
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
  const setting_type<std::vector<bible::translation>> translations;
};

///
/// Workflow bible reference ocr.
///
class workflow_bible_reference_ocr final
{
public: // Typedefs
  using settings_type = workflow_bible_reference_ocr_settings::sptr_type;
  using language = core::core_tesseract_common::language;

public: // Structors
  workflow_bible_reference_ocr(language language);
  ~workflow_bible_reference_ocr() noexcept;

public: // Modifiers
  auto run_once(const settings_type& settings) -> void;

private: // Typedefs
  using screen_rect_type = util::screen_types::screen_rect_type;
  using screen_coordinates_type = util::screen_types::screen_coordinates_type;
  using tesseract_choice = core::core_tesseract_common::tesseract_choice;
  using tesseract_choices = core::core_tesseract_common::tesseract_choices;
  using character_data = core::core_bible_reference_ocr_common::character_data;
  using reference_position_data = core::core_bible_reference_ocr_common::reference_position_data;

  ///
  /// Internal data structure.
  ///
  struct data_t final
  {
    screen_coordinates_type current_cursor_position{0, 0};
    std::optional<std::int32_t> current_char_height{};
    std::vector<screen_rect_type> capture_areas{};
  };

private: // Implementation
  auto find_references() -> void;
  auto capture_img(const screen_rect_type& rect) -> bool;
  auto parse_tesseract_recognition(const screen_rect_type& image_dimensions, const screen_coordinates_type& relative_cursor_pos)
    -> std::optional<std::vector<bible::reference_range>>;
  auto get_reference_position_main(const screen_coordinates_type& relative_cursor_pos)
    -> std::optional<reference_position_data>;
  auto get_reference_position_choices(const screen_coordinates_type& relative_cursor_pos)
    -> std::vector<reference_position_data>;
  auto get_min_distance_index(const std::vector<character_data>& char_data) -> std::optional<std::size_t>;

private: // Variables
  const app_framework::thread_pool::strand_id_type strand_id_{app_framework::thread_pool::strand_id()};
  const std::unique_ptr<core::core_bible_reference_ocr> core_bible_reference_ocr_;
  const std::unique_ptr<core::core_bible_reference> core_bible_reference_;
  const std::unique_ptr<core::core_bibleserver_lookup> core_bibleserver_lookup_;
  const std::unique_ptr<core::core_tesseract> core_tesseract_;

  settings_type settings_{nullptr};
  data_t data_;
};

} // namespace bibstd::workflow
