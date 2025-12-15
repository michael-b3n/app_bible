#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/core/core_tesseract_common.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/util/screen_types.hpp"
#include "bibstd/workflow/workflow_base.hpp"

#include <memory>
#include <stop_token>
#include <vector>

namespace bibstd::core
{
// Forward declarations
class core_bible_ref_ocr;
class core_bible_ref;
class core_bibleserver_lookup;
} // namespace bibstd::core

namespace bibstd::workflow
{

// Forward declarations
class workflow_bible_ref_ocr;

namespace detail
{

///
/// Available signal IDs.
///
enum class workflow_bible_ref_ocr_signal_id
{
  ended
};

///
/// Start parameters for workflow bible reference ocr.
///
struct workflow_bible_ref_ocr_start_params final
{
  util::screen_coordinates_type cursor_position{0, 0};
};

///
/// Expected result type for workflow bible reference ocr.
///
using workflow_bible_ref_ocr_expected_result_type = std::vector<bible::reference_range>;

///
/// Base type definition.
///
using workflow_bible_ref_ocr_base_type = workflow_base<
  workflow_bible_ref_ocr,                       // derived workflow
  workflow_bible_ref_ocr_start_params,          // start params type,
  workflow_bible_ref_ocr_expected_result_type>; // expected result type

} // namespace detail

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
  const setting_type<core::core_tesseract_common::language> language;
  const setting_type<std::vector<bible::translation>> translations;
  const setting_type<bool> recognize_largest_bounding_box;
};

///
/// Bible reference ocr: this workflow searches for bible references on a screen area using OCR around the cursor position.
/// Signal IDs to connect to:
/// - started: Emitted when the OCR process starts. Slots receive the start parameters `start_params`.
/// - ended: Emitted when the OCR process ends. Slots receive the result parameters `result_type`.
///
class workflow_bible_ref_ocr final
  : public detail::workflow_bible_ref_ocr_base_type
  , public framework::settings_owner<workflow_bible_ref_ocr_settings>
  , public signal::adapter<signal::named_signal<
      detail::workflow_bible_ref_ocr_signal_id::ended,
      signal::signal_type<void(const detail::workflow_bible_ref_ocr_base_type::result_type&)>>>
{
public: // Typedefs
  using start_params = detail::workflow_bible_ref_ocr_base_type::start_params;
  using result_type = detail::workflow_bible_ref_ocr_base_type::result_type;

public: // Structors
  workflow_bible_ref_ocr();
  ~workflow_bible_ref_ocr() noexcept;

public: // Modifiers
  ///
  /// Search bible references on a screen area using OCR around the cursor position.
  /// \param cursor_position Position of the cursor on the screen
  /// \return start result containing a process ID and a stop source for stopping the search
  ///
  auto start(const start_params& params) -> std::stop_source;

private: // Typedefs
  using screen_rect_type = util::screen_rect_type;

private: // Implementation
  auto find_references(
    const std::shared_ptr<core::core_bible_ref_ocr>& core_bible_ref_ocr,
    auto&& image_data,
    bool recognize_largest_bounding_box,
    std::stop_token stop_token
  ) -> decltype(result_type::result);
  auto parse_tesseract_recognition(
    const std::shared_ptr<core::core_bible_ref_ocr>& core_bible_ref_ocr,
    const util::screen_coordinates_type& relative_cursor_pos
  ) -> std::vector<bible::reference_range>;

private: // Variables
  const framework::thread_pool::strand_id_type strand_id_{framework::thread_pool::strand_id()};
  const std::unique_ptr<core::core_bible_ref> core_bible_ref_;
  const std::unique_ptr<core::core_bibleserver_lookup> core_bibleserver_lookup_;
  std::atomic<std::shared_ptr<core::core_bible_ref_ocr>> core_bible_ref_ocr_;
};

} // namespace bibstd::workflow
