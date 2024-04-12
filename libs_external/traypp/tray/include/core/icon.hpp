#pragma once
#include <string>
#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#endif

namespace Tray
{

#if defined(__linux__)
class Icon
{
  std::string iconPath;

public:
  Icon(std::string path);
  Icon(const char* path);
  operator const char*();
};
#elif defined(_WIN32)
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
#endif

} // namespace Tray
