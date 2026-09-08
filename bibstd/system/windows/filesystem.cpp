#include "bibstd/system/filesystem.hpp"
#include "bibstd/util/exception.hpp"

#include <filesystem>

namespace bibstd::system
{

///
///
auto filesystem::local_data_folder() -> std::filesystem::path
{
  const auto* const appdata = std::getenv("LOCALAPPDATA");
  if(appdata == nullptr)
  {
    throw util::exception("local appdata not found");
  }
  return std::filesystem::path(appdata) / executable_location().stem();
}

} // namespace bibstd::system
