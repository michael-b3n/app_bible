#include "bibstd/system/open_browser.hpp"
#include "bibstd/system/windows/win.hpp"
#include "bibstd/util/exception.hpp"

#include <combaseapi.h>
#include <shellapi.h>

namespace bibstd::system
{

///
///
auto open_browser::open(const std::string& url) -> bool
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
  const auto success = ShellExecuteExW(&sh_exec_info);
  const auto wait_result = WaitForSingleObject(sh_exec_info.hProcess, 1000 /*ms*/);
  const auto no_timeout = wait_result != WAIT_TIMEOUT;
  if(sh_exec_info.hProcess)
  {
    if(!no_timeout)
    {
      TerminateProcess(sh_exec_info.hProcess, 0);
    }
    CloseHandle(sh_exec_info.hProcess);
  }
  CoUninitialize();
  return success && no_timeout;
}

} // namespace bibstd::system
