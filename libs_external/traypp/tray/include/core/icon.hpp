#pragma once

#include <string>
#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>

namespace Tray
{

///
/// Windows icon class.
///
class Icon
{
  HICON hIcon;

public:
  Icon(HICON icon);
  Icon(WORD resource);
  Icon(const char* path);
  Icon(const std::string& path);

  operator HICON();
};

} // namespace Tray

#endif
