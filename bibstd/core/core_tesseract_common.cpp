#include "bibstd/core/core_tesseract_common.hpp"
#include "bibstd/system/filesystem.hpp"

#include <ranges>
#include <string_view>

namespace bibstd::core
{

///
///
auto core_tesseract_common::tessdata_folder_finder() -> std::optional<std::filesystem::path>
{
  const auto executable_folder_parent = system::filesystem::executable_folder().parent_path();
  const auto root = executable_folder_parent.parent_path();
  const auto best_guess = executable_folder_parent / "share" / "tessdata";
  if(std::filesystem::exists(best_guess))
  {
    return best_guess;
  }
  auto result = std::optional<std::filesystem::path>{};
  const auto is_tessdata_folder = [](const auto& e)
  { return e.is_directory() && e.path().filename() == std::string_view{"tessdata"}; };
  const auto search_folder_from = [&](const auto& root)
  {
    constexpr auto max_search_iterations = 1024;
    auto counter = std::size_t{0};
    auto continue_condition = [&]([[maybe_unused]] const auto&) { return !result || counter++ < max_search_iterations; };
    for(const auto& entry :
        std::filesystem::recursive_directory_iterator{root, std::filesystem::directory_options::skip_permission_denied} |
          std::views::filter(is_tessdata_folder) | std::views::take_while(continue_condition))
    {
      result = entry.path();
    }
  };
  search_folder_from(executable_folder_parent);
  if(!result)
  {
    search_folder_from(root);
  }
  return result;
}

} // namespace bibstd::core
