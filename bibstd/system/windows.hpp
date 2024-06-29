#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10
#define WINVER       0x0A00 // Windows 10
#include <windows.h>
