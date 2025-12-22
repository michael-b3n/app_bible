#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/framework/runtime_uid.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/framework/settings_owner.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/signal/common.hpp"
#include "bibstd/system/hotkey_common.hpp"
#include "bibstd/util/screen_types.hpp"

#include <memory>
#include <stop_token>

namespace bibstd::workflow
{
class workflow_bible_ref_ocr;
} // namespace bibstd::workflow
namespace bibstd::visual
{
class widget_main;
} // namespace bibstd::visual
namespace bibstd::presenter
{
namespace detail
{

///
/// Available signal IDs.
///
enum class presenter_bible_ref_ocr_signal_id
{
  started,
  ended
};

} // namespace detail

///
/// Settings corresponding to presenter bible reference ocr.
/// Presenter settings shall only be responsible for client interaction.
///
class presenter_bible_ref_ocr_settings final : public framework::settings_base
{
public: // Structors
  presenter_bible_ref_ocr_settings();
  ~presenter_bible_ref_ocr_settings() noexcept = default;

public: // Variables
  const setting_type<system::hotkey_common::key_modifier> hotkey_modifier;
  const setting_type<system::hotkey_common::key> hotkey;
};

///
/// Presenter bible reference ocr.
/// Signals:
/// - queued: Emitted when the OCR is started. Slots receive (int x, int y).
/// - started: Emitted when the OCR process starts.
/// - ended: Emitted when the OCR process ends.
///
class presenter_bible_ref_ocr final
  : public framework::settings_owner<presenter_bible_ref_ocr_settings>
  , public signal::adapter<
      signal::named_signal<
        detail::presenter_bible_ref_ocr_signal_id::started,
        signal::signal_type<void(framework::runtime_uid_type)>>,
      signal::named_signal<
        detail::presenter_bible_ref_ocr_signal_id::ended,
        signal::signal_type<void(framework::runtime_uid_type, std::vector<bible::reference_range>)>>>
{
public: // Structors
  presenter_bible_ref_ocr();
  ~presenter_bible_ref_ocr() noexcept;

public: // Operations
  ///
  /// Start the bible reference recognition.
  /// \param position The position on the screen where to start the recognition.
  /// \return stop source to cancel the recognition
  ///
  auto start(const util::screen_coordinates_type& position) -> std::stop_source;

private: // Variables
  std::unique_ptr<workflow::workflow_bible_ref_ocr> workflow_bible_ref_ocr_;
  signal::connection_store connections_;
};

} // namespace bibstd::presenter
