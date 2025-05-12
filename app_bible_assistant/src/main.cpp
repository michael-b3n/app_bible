///
/// Main file.
///

#include "src/version.hpp"

#include <app_framework/active_worker.hpp>
#include <app_framework/main_loop.hpp>
#include <system/filesystem.hpp>
#include <system/hotkey.hpp>
#include <system/tray.hpp>
#include <util/date.hpp>
#include <util/incbin.hpp>
#include <util/log.hpp>
#include <util/string.hpp>

#include <workflow/workflow_bible_reference_ocr.hpp>

#include <filesystem>
#include <format>

INC_RESOURCE(icon, "res/icon.ico");
const auto icon_view = bibstd::util::incbin::to_span<std::byte>(res_icon_data, res_icon_size);

///
/// Main function.
///
int main()
{
  const auto logger = bibstd::util::logger();
  LOG_INFO("executable: {}", bibstd::system::filesystem::executable_location().string());
  LOG_INFO("version: {}", bible_assistant::version::version_string);
  LOG_INFO("commit_hash: {}", bible_assistant::version::commit_hash);
  LOG_INFO("commit_date: {}", bible_assistant::version::commit_date);

  // Init backend
  auto workflow_reference_finder =
    bibstd::workflow::workflow_bible_reference_ocr(bibstd::workflow::workflow_bible_reference_ocr::language::de);

  // Init settings
  auto workflow_reference_finder_settings = std::make_shared<bibstd::workflow::workflow_bible_reference_ocr_settings>();

  // Start system hotkey manager.
  const auto hotkey_guard = bibstd::system::hotkey::init();
  const auto pool_guard = bibstd::app_framework::thread_pool::init();

  const auto do_on_exit = [&]() { bibstd::app_framework::main_loop::exit(); };

  // Start system tray.
  const auto tray_guard = bibstd::system::tray::init(
    bibstd::system::tray::icon_buffer{icon_view},
    {
      bibstd::system::tray::entry_type{bibstd::system::tray::button{"Exit", do_on_exit}},
      // ...
    }
  );

  // Register hotkeys
  bibstd::system::hotkey::register_callback(
    bibstd::system::hotkey::key::vk_f,
    bibstd::system::hotkey::key_modifier::alt,
    [&]() { workflow_reference_finder.find_references(workflow_reference_finder_settings); }
  );

  // Enter main loop.
  bibstd::app_framework::main_loop::run();

  LOG_INFO("main", "Exit application: {}", bibstd::util::format_current_time_CET());
  return EXIT_SUCCESS;
}
