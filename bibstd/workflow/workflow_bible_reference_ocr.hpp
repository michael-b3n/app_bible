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
  const setting_type<std::uint16_t> assumed_initial_char_height;
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
  auto find_references(const settings_type& settings) -> void;

private: // Typedefs
  using screen_rect_type = util::screen_types::screen_rect_type;
  using screen_coordinates_type = util::screen_types::screen_coordinates_type;
  using parse_result_type = std::pair<bool, std::vector<bible::reference_range>>;

private: // Implementation
  auto find_references_impl(const screen_coordinates_type& cursor_position) -> parse_result_type;
  auto parse_tesseract_recognition(const screen_rect_type& image_dimensions, const screen_coordinates_type& relative_cursor_pos)
    -> parse_result_type;

private: // Variables
  const app_framework::thread_pool::strand_id_type strand_id_{app_framework::thread_pool::strand_id()};
  const std::unique_ptr<core::core_bible_reference_ocr> core_bible_reference_ocr_;
  const std::unique_ptr<core::core_bible_reference> core_bible_reference_;
  const std::unique_ptr<core::core_bibleserver_lookup> core_bibleserver_lookup_;

  settings_type settings_{nullptr};
};

} // namespace bibstd::workflow
