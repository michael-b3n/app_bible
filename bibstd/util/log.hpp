#pragma once

#include <filesystem>
#include <format>
#include <string_view>

namespace bibstd::util
{

///
/// List of all available logger levels.
///
enum class logger_level
{
  debug,
  info,
  warning,
  error
};

///
/// Get the global log level.
/// \return global log level
///
auto global_log_level() -> logger_level;

///
/// Log message with debug level.
/// \param msg String view message
///
auto log_debug(std::string_view&& msg) -> void;

///
/// Log message with info level.
/// \param msg String view message
///
auto log_info(std::string_view&& msg) -> void;

///
/// Log message with warning level.
/// \param msg String view message
///
auto log_warn(std::string_view&& msg) -> void;

///
/// Log message with error level.
/// \param msg String view message
///
auto log_error(std::string_view&& msg) -> void;

///
/// Init logger and shutdown logger with RAII.
///
struct logger final
{
  logger();
  ~logger() noexcept;
};

} // namespace bibstd::util

///
/// Internal std formatted string helper macro.
///
// clang-format off
#define INT_LOG_INTERNAL_FMT_STR(CHANNEL, LEVEL, FMT_STR, ...)                                                                                       \
  {                                                                                                                                                  \
    if(LEVEL >= ::bibstd::util::global_log_level())                                                                                                  \
    {                                                                                                                                                \
      try                                                                                                                                            \
      {                                                                                                                                              \
        const auto log_string = std::format("[{}] " FMT_STR, CHANNEL, __VA_ARGS__);                                                                  \
        if      constexpr(LEVEL == ::bibstd::util::logger_level::debug)   { ::bibstd::util::log_debug(log_string); }                                 \
        else if constexpr(LEVEL == ::bibstd::util::logger_level::info)    { ::bibstd::util::log_info(log_string);  }                                 \
        else if constexpr(LEVEL == ::bibstd::util::logger_level::warning) { ::bibstd::util::log_warn(log_string);  }                                 \
        else if constexpr(LEVEL == ::bibstd::util::logger_level::error)   { ::bibstd::util::log_error(log_string); }                                 \
      }                                                                                                                                              \
      catch(std::format_error& exception)                                                                                                            \
      {                                                                                                                                              \
        ::bibstd::util::log_error(std::format("INTERNAL_FMT_STR format error: {}.", exception.what()));                                              \
      }                                                                                                                                              \
    }                                                                                                                                                \
  }
// clang-format on

///
/// Default logging macros
///

#define LOG_DEBUG(CHANNEL, FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(CHANNEL, ::bibstd::util::logger_level::debug, FMT_STR, __VA_ARGS__);
#define LOG_INFO(CHANNEL, FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(CHANNEL, ::bibstd::util::logger_level::info, FMT_STR, __VA_ARGS__);
#define LOG_WARN(CHANNEL, FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(CHANNEL, ::bibstd::util::logger_level::warning, FMT_STR, __VA_ARGS__);
#define LOG_ERROR(CHANNEL, FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(CHANNEL, ::bibstd::util::logger_level::error, FMT_STR, __VA_ARGS__);
