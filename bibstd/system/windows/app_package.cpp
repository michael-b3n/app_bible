#include "bibstd/system/app_package.hpp"

#include "bibstd/system/windows/win.hpp"

#include <appmodel.h>

namespace bibstd::system
{

///
///
auto app_package::packaged() -> bool
{
  // The package identity of a process never changes, so ask the api once.
  static const auto result = []
  {
    auto length = UINT32{0};
    return GetCurrentPackageFamilyName(&length, nullptr) != APPMODEL_ERROR_NO_PACKAGE;
  }();
  return result;
}

} // namespace bibstd::system
