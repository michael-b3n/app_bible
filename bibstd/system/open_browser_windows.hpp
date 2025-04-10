#pragma once

#include "system/windows.hpp"
#include "util/exception.hpp"

#include <combaseapi.h>
#include <shellapi.h>

#include <string_view>

namespace bibstd::system
{

///
/// Filesystem class for windows implementation.
///
struct open_browser final
{
  ///
  /// Open URL in the default web browser.
  /// \return true if successful, false otherwise
  ///
  static inline auto open(const std::string& url) -> bool;
};

///
///
inline auto open_browser::open(const std::string& url) -> bool
{
  const int wchar_count = MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, NULL, 0);
  std::wstring url_wstr(wchar_count, L'\0');

  MultiByteToWideChar(CP_UTF8, 0, url.c_str(), -1, url_wstr.data(), wchar_count);

  SHELLEXECUTEINFOW sh_exec_info = {sizeof(sh_exec_info)};
  sh_exec_info.cbSize = sizeof(SHELLEXECUTEINFO);
  sh_exec_info.fMask = SEE_MASK_NOCLOSEPROCESS;
  sh_exec_info.hwnd = NULL;
  sh_exec_info.lpVerb = L"open";
  sh_exec_info.lpFile = url_wstr.c_str();
  sh_exec_info.lpParameters = L"";
  sh_exec_info.lpDirectory = NULL;
  sh_exec_info.nShow = SW_SHOWDEFAULT;
  sh_exec_info.hInstApp = NULL;

  /// \see https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecuteexa
  CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
  const auto success = !ShellExecuteExW(&sh_exec_info);
  const auto wait_result = WaitForSingleObject(sh_exec_info.hProcess, 1000 /*ms*/);
  const auto timeout = wait_result != WAIT_TIMEOUT;
  if(sh_exec_info.hProcess)
  {
    if(timeout)
    {
      TerminateProcess(sh_exec_info.hProcess, 0);
    }
    CloseHandle(sh_exec_info.hProcess);
  }
  CoUninitialize();
  return success && !timeout;
}

} // namespace bibstd::system
