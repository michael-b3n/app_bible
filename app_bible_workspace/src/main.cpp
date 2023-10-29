///
/// Main file.
///

#include <system/filesystem.hpp>
#include <util/date.hpp>
#include <util/log.hpp>
#include <util/string_view.hpp>

#include <bible/bible_book_name_variants_de.hpp>
#include <io/key_input_handler.hpp>

#include <filesystem>
#include <format>

#include <webui.hpp>

///
/// Main function.
///
int main()
{
  const auto logger = bibstd::util::logger();
  LOG_INFO("main", "Executable: {}", bibstd::system::filesystem::executable_location().string());

  // auto key_input = bibstd::io::key_input_handler();
  // key_input.register_global_key(
  //   0x46, bibstd::io::key_input_handler::key_modifier::alt, []() { bibstd::util::log_debug("PRESSED!"); });
  [[maybe_unused]] constexpr auto name_variant_list = bibstd::bible::bible_book_name_variants_de::name_variants_list;

  // Create a new window
  webui::window my_window;

  // Show a new window
  my_window.show_browser("<html><head><script src=\"webui.js\"></script></head> C++ Hello World ! </html>", NoBrowser);
  const auto current_url = my_window.get_url();
  std::system(std::format("start microsoft-edge:{}", current_url).data());

  // Wait until all windows get closed
  webui::wait();
  LOG_INFO("main", "Exit application: {}", bibstd::util::format_current_time_CET());
  return 0;
}
