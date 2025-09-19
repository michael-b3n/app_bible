#pragma once

#include "framework/settings_base.hpp"
#include "framework/settings_owner.hpp"
#include "util/signals.hpp"

#include <mutex>

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
  const setting_type<std::pair<system::hotkey_common::key_modifier, system::hotkey_common::key>> hotkey;
};

///
/// Presenter bible reference ocr.
///
class presenter_bible_ref_ocr final : public framework::settings_owner<presenter_bible_ref_ocr_settings>
{
public: // Typedefs

public: // Structors
  presenter_bible_ref_ocr();
  ~presenter_bible_ref_ocr() noexcept;

public: // Modifiers

private: // Implementation

private: // Variables
  std::unique_ptr<workflow::workflow_bible_ref_ocr> workflow_bible_ref_ocr_;
  util::connection_store connections_;
};

} // namespace bibstd::presenter
