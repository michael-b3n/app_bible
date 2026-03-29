#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/util/language.hpp"
#include "bibstd/util/screen_types.hpp"
#include "bibstd/workflow/workflow_base.hpp"
#include "bibstd/workflow/workflow_hotkey.hpp"
#include "bibstd/workflow/workflow_scripture.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace bibstd::core
{
// Forward declarations
class core_bible_ref_ocr;
class core_bible_ref_finder;
} // namespace bibstd::core

namespace bibstd::workflow
{
// Forward declarations
class workflow_scripture;

///
/// Settings corresponding to workflow bible reference ocr.
///
class workflow_bible_ref_ocr_settings final : public framework::settings_base
{
public: // Structors
  workflow_bible_ref_ocr_settings();
  ~workflow_bible_ref_ocr_settings() noexcept = default;

public: // Variables
  const setting_type<std::optional<std::filesystem::path>> tessdata_path;
  const setting_type<util::language> language;
  const setting_type<bool> recognize_largest_bounding_box;
};

///
/// Bible reference ocr: this workflow searches for bible references on a screen area using OCR around the cursor position.
///
class workflow_bible_ref_ocr final
  : public workflow_base<workflow_bible_ref_ocr>
  , public framework::settings_owner<workflow_bible_ref_ocr_settings>
{
public: // Typedefs
  struct params final
  {
    util::screen_coordinates_type cursor_position{0, 0};
  };
  struct result final
  {
    std::vector<bible::reference_range> reference_ranges;
    std::optional<bible::parser::html_passage> passage;
  };

  using process_params = framework::process_params<params>;
  using process_result = framework::process_result<result>;

public: // Structors
  ///
  /// Constructor for workflow_bible_ref_ocr.
  /// \param workflow_scripture scripture workflow for accessing scripture data
  /// If tesseract is used for OCR, a valid tessdata path is required \see core_tesseract_common::tessdata_folder_finder.
  ///
  workflow_bible_ref_ocr(std::shared_ptr<workflow_scripture> workflow_scripture);
  ~workflow_bible_ref_ocr() noexcept;

public: // Modifiers
  ///
  /// Search bible references on a screen area using OCR around the cursor position.
  /// \param params Process parameters containing the cursor position
  /// \return list of found bible reference ranges, or an unexpected result in case of failure
  ///
  auto find(const process_params& params) -> process_result;

private: // Typedefs
  using screen_rect_type = util::screen_rect_type;

  struct settings_local final
  {
    util::language language;
    bool recognize_largest_bounding_box;
  };

private: // Implementation
  auto find_references(
    const std::shared_ptr<core::core_bible_ref_ocr>& core_bible_ref_ocr, auto&& image_data, const settings_local& local_settings
  ) -> framework::process_result<std::vector<bible::reference_range>>;
  auto parse_tesseract_recognition(
    const std::shared_ptr<core::core_bible_ref_ocr>& core_bible_ref_ocr,
    const util::screen_coordinates_type& relative_cursor_pos,
    const settings_local& local_settings
  ) -> std::vector<bible::reference_range>;

private: // Variables
  mutable std::mutex mtx_;
  const std::unique_ptr<core::core_bible_ref_finder> core_bible_ref_finder_;
  std::atomic<std::shared_ptr<core::core_bible_ref_ocr>> core_bible_ref_ocr_;
  const std::shared_ptr<workflow_scripture> workflow_scripture_;
};

} // namespace bibstd::workflow
