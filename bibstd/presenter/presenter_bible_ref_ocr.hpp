#pragma once

#include "bibstd/bible/parser.hpp"
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
class workflow_bible_ref_lookup;
class workflow_bible_ref_ocr;
class workflow_scripture;
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
  found,
  lookup_ended
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
/// - started: Emitted when the OCR process starts.
/// - found: Emitted when the OCR finished searching for references.
/// - lookup_ended: Emitted when the reference lookup process ends.
///
class presenter_bible_ref_ocr final
  : public framework::settings_owner<presenter_bible_ref_ocr_settings>
  , public signal::adapter<
      signal::named_signal<
        detail::presenter_bible_ref_ocr_signal_id::started,
        signal::signal_type<void(framework::runtime_uid_type)>>,
      signal::named_signal<
        detail::presenter_bible_ref_ocr_signal_id::found,
        signal::signal_type<void(framework::runtime_uid_type, std::optional<bible::parser::html_passage>)>>,
      signal::named_signal<
        detail::presenter_bible_ref_ocr_signal_id::lookup_ended,
        signal::signal_type<void(framework::runtime_uid_type)>>>
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
  std::unique_ptr<workflow::workflow_bible_ref_lookup> workflow_bible_ref_lookup_;
  std::unique_ptr<workflow::workflow_bible_ref_ocr> workflow_bible_ref_ocr_;
  std::unique_ptr<workflow::workflow_scripture> workflow_scripture_;
  signal::connection_store connections_;
};

} // namespace bibstd::presenter
