#include "util/timer.hpp"
#include "util/log.hpp"

#include <format>

namespace bibstd::util
{

///
///
timer::timer()
  : time_point_{std::chrono::system_clock::now()}
{
}

///
///
auto timer::current_duration() const -> std::chrono::milliseconds
{
  const auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - time_point_);
}

///
///
scoped_timer_logger::scoped_timer_logger(const std::string_view function_name)
  : function_name_{function_name}
{
}

///
///
scoped_timer_logger::~scoped_timer_logger()
{
  const auto duration = timer_.current_duration();
  LOG_INFO("Measured scoped time duration: {} ms | {}", duration.count(), function_name_);
}

} // namespace bibstd::util
