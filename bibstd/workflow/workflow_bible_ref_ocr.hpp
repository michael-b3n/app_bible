#pragma once

#include "bibstd/bible/reference.hpp"
#include "bibstd/bible/reference_ocr.hpp"
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

// Forward declarations
namespace bibstd::core
{
class core_bible_ref_ocr;
class core_bible_ref_finder;
} // namespace bibstd::core

namespace bibstd::workflow
{

///
/// Settings corresponding to workflow bible reference ocr.
///
struct workflow_bible_ref_ocr_settings final : public framework::settings_base
{
  // Structors
  workflow_bible_ref_ocr_settings();

  // Variables
  const setting_type<std::optional<std::filesystem::path>> tessdata_path;
  const setting_type<std::optional<std::string>> character_recognition_ocr_engine;
  const setting_type<std::optional<std::string>> layout_recognition_ocr_engine;
  const setting_type<util::language> language;
  const setting_type<std::string> fallback_versification_name;
};

///
/// Bible reference ocr: this workflow searches for bible references on a screen area using OCR around the cursor position.
///
class workflow_bible_ref_ocr final
  : public workflow_base<workflow_bible_ref_ocr>
  , public framework::settings_owner<workflow_bible_ref_ocr_settings>
{
  // Typedefs
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

  ///
  /// Result of capturing a screen area.
  /// This contains the relative cursor position and the captured pixel plane.
  ///
  struct capture_screen_result_t final
  {
    util::screen_coordinates_type relative_cursor_pos;
    util::pixel_plane_type image;
  };

  ///
  /// Local settings for the find_references function.
  /// The settings are a partial snapshot of the workflow settings.
  ///
  struct settings_t final
  {
    std::string character_recognition_ocr_engine;
    std::optional<std::string> layout_recognition_ocr_engine;
    util::language language;
    workflow_scripture::versification_wrapper_type versification;
  };

public: // Typedefs
  using params = framework::process_params<params_t>;
  using result = framework::process_result<result_t>;

public: // Structors
  ///
  /// Constructor for workflow_bible_ref_ocr.
  /// If tesseract is used for OCR, a valid tessdata path is required:
  /// \see txt::ocr_engine_tesseract::tessdata_folder_finder.
  ///
  workflow_bible_ref_ocr(std::shared_ptr<workflow_scripture> workflow_scripture);
  ~workflow_bible_ref_ocr() noexcept;

public: // Modifiers
  ///
  /// Search bible references on a screen area using OCR around the cursor position.
  /// \param params Process parameters containing the cursor position
  /// \return list of found bible reference ranges, or an unexpected result in case of failure
  ///
  [[nodiscard]] auto find(const params& params) -> result;

private: // Implementation
  auto init() -> void;
  [[nodiscard]] auto capture_screen(const util::screen_coordinates_type& cursor_position) const
    -> std::optional<capture_screen_result_t>;
  [[nodiscard]] auto versification() const -> decltype(settings_t::versification);
  [[nodiscard]] auto find_references(auto&& image_data, const settings_t& settings)
    -> framework::process_result<std::vector<bible::reference_range>>;

private: // Variables
  mutable std::mutex mtx_;
  const std::unique_ptr<core::core_bible_ref_finder> core_bible_ref_finder_;
  const std::shared_ptr<workflow_scripture> workflow_scripture_;
  bible::reference_ocr::ocr_engine_list_type ocr_engines_;
};

} // namespace bibstd::workflow
