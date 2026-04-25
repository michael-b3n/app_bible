#pragma once

#include "bibstd/bible/reference.hpp"
#include "bibstd/bible/reference_range.hpp"
#include "bibstd/bible/scripture.hpp"
#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/util/language.hpp"
#include "bibstd/util/screen_types.hpp"
#include "bibstd/workflow/workflow_base.hpp"
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
  const setting_type<std::string> fallback_versification_name;
};

///
/// Bible reference ocr: this workflow searches for bible references on a screen area using OCR around the cursor position.
///
class workflow_bible_ref_ocr final
  : public workflow_base<workflow_bible_ref_ocr>
  , public framework::settings_owner<workflow_bible_ref_ocr_settings>
{
private: // Typedefs
  struct params_t final
  {
    util::screen_coordinates_type cursor_position{0, 0};
  };

  ///
  /// Result of bible reference ocr process.
  /// This contains the found reference ranges, the first reference found
  /// and the passage content for the first reference found.
  ///
  struct result_t final
  {
    std::vector<bible::reference_range> reference_ranges;
    std::optional<bible::reference> first_reference;
    std::optional<bible::scripture::passage_html_type> passage;
  };

public: // Typedefs
  using params = framework::process_params<params_t>;
  using result = framework::process_result<result_t>;

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
  auto find(const params& params) -> result;

private: // Typedefs
  using screen_rect_type = util::screen_rect_type;
  using versification_wrapper_type = workflow_scripture::versification_wrapper_type;

  struct settings_local final
  {
    util::language language;
    bool recognize_largest_bounding_box;
    versification_wrapper_type versification;
  };

private: // Implementation
  auto versification() const -> versification_wrapper_type;
  auto find_references(auto&& image_data, const settings_local& local_settings)
    -> framework::process_result<std::vector<bible::reference_range>>;
  auto parse_recognition(const util::screen_coordinates_type& relative_cursor_pos, const settings_local& local_settings)
    -> std::vector<bible::reference_range>;

private: // Variables
  mutable std::mutex mtx_;
  const std::unique_ptr<core::core_bible_ref_finder> core_bible_ref_finder_;
  std::shared_ptr<core::core_bible_ref_ocr> core_bible_ref_ocr_;
  const std::shared_ptr<workflow_scripture> workflow_scripture_;
};

} // namespace bibstd::workflow
