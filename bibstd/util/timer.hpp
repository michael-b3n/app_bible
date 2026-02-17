#pragma once

#include <boost/preprocessor/cat.hpp>

#include <chrono>
#include <source_location>

namespace bibstd::util
{

///
/// Timer class used for measuring time durations.
///
class timer final
{
public: // Constructor
  timer();

public: // Accessors
  ///
  /// Get current duration from constructed timepoint to timepoint of the call of this function.
  /// \return current duration
  ///
  [[nodiscard]] auto current_duration() const -> std::chrono::milliseconds;

private: // Variables
  std::chrono::system_clock::time_point time_point_;
};

///
/// Scoped timer logging duration on destruction.
///
class scoped_timer_logger final
{
public: // Destructor
  scoped_timer_logger(std::source_location&& source_location = std::source_location::current());
  ~scoped_timer_logger();

private: // Variables
  const std::source_location source_location_;
  const timer timer_;
};

} // namespace bibstd::util

///
/// Scoped timer logging macro.
///
#define SCOPED_TIMER_LOG()                                                                                                     \
  [[maybe_unused]] const auto BOOST_PP_CAT(__scoped_timer_logger_instance, __COUNTER__) = ::bibstd::util::scoped_timer_logger();
