#pragma once

#include "app_framework/settings_base.hpp"
#include "app_framework/thread_pool.hpp"
#include "core/core_tesseract_common.hpp"
#include "math/value_range.hpp"
#include "system/screen.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <string_view>
#include <vector>

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
  using bounding_box_type = core::core_tesseract_common::bounding_box_type;

public: // Structors
  workflow_bible_reference_ocr(language language);
  ~workflow_bible_reference_ocr() noexcept;

public: // Modifiers
  auto run_once(const settings_type& settings) -> void;

private: // Typedefs
  using screen_rect_type = system::screen::screen_rect_type;
  using screen_coordinates_type = system::screen::screen_coordinates_type;
  using index_range_type = math::value_range<std::size_t>;

private: // Constants
  static constexpr auto capture_ocr_area_steps = std::array{1, 2, 4};
  static constexpr auto vertical_range_denominator = 32;
  static constexpr auto height_to_width_ratio = 8;

  ///
  /// Internal data structure.
  ///
  struct data_t final
  {
    screen_coordinates_type current_cursor_position{0, 0};
    std::optional<std::int32_t> current_char_height{};
    std::vector<screen_rect_type> capture_areas{};
  };

  struct character_data final
  {
    double distance;
    bounding_box_type bounding_box;
  };

  struct reference_position_data final
  {
    std::string text;
    std::vector<character_data> char_data;
    std::size_t index;
  };

private: // Implementation
  auto find_references() -> void;
  auto set_capture_areas() -> bool;
  auto capture_img(const screen_rect_type& rect) -> bool;
  auto get_reference_position_data(
    const screen_coordinates_type& relative_cursor_position,
    std::string_view text_paragraph,
    const bounding_box_type& bounding_box
  ) -> std::optional<reference_position_data>;
  auto is_valid_capture_area(
    const bounding_box_type& image_dimensions, const reference_position_data& reference_position, index_range_type index_range
  ) -> bool;

private: // Variables
  const std::unique_ptr<core::core_bible_reference> core_bible_reference_;
  const std::unique_ptr<core::core_bibleserver_lookup> core_bibleserver_lookup_;
  const std::unique_ptr<core::core_tesseract> core_tesseract_;

  const app_framework::thread_pool::strand_id_type strand_id_{app_framework::thread_pool::strand_id()};
  settings_type settings_{nullptr};
  data_t data_;
};

} // namespace bibstd::workflow
