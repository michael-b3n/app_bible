#include "system/environment_variable.hpp"
#include "system/windows/win.hpp"
#include "util/log.hpp"

namespace bibstd::system
{

///
///
auto environment_variable::set(const std::string& name, const std::string& value) -> bool
{
  if(name.empty())
  {
    return false;
  }
  const bool result = SetEnvironmentVariableA(name.c_str(), value.empty() ? nullptr : value.c_str());
  LOG_INFO("set environment variable {}: name=\"{}\", value=\"{}\"", result ? "succeeded" : "failed", name, value);
  return result;
}

} // namespace bibstd::system
