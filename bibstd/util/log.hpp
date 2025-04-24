#pragma once

#include <filesystem>
#include <format>
#include <source_location>
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
#define INT_LOG_INTERNAL_FMT_STR(LEVEL, FMT_STR, ...)                                                                                                                                                                   \
  {                                                                                                                                                                                                                     \
    if(LEVEL >= ::bibstd::util::global_log_level())                                                                                                                                                                     \
    {                                                                                                                                                                                                                   \
      constexpr std::string_view __log_path = std::source_location::current().file_name();                                                                                                                              \
      constexpr auto __log_last_slash_pos = __log_path.find_last_of("/\\");                                                                                                                                             \
      constexpr std::string_view __log_file_name_with_ext = (__log_last_slash_pos != std::string_view::npos) ? __log_path.substr(__log_last_slash_pos + 1) : __log_path;                                                \
      constexpr auto __log_dot_pos = __log_file_name_with_ext.find_last_of('.');                                                                                                                                        \
      constexpr std::string_view __log_file_name = (__log_dot_pos != std::string_view::npos) ? __log_file_name_with_ext.substr(0, __log_dot_pos) : __log_file_name_with_ext;                                            \
      constexpr auto __log_parent_folder_end = (__log_last_slash_pos != std::string_view::npos) ? __log_path.substr(0, __log_last_slash_pos).find_last_of("/\\") : std::string_view::npos;                              \
      constexpr std::string_view __log_folder = (__log_parent_folder_end != std::string_view::npos) ? __log_path.substr(__log_parent_folder_end + 1, __log_last_slash_pos - (__log_parent_folder_end + 1)) : "unknown"; \
      try                                                                                                                                                                                                               \
      {                                                                                                                                                                                                                 \
        const auto log_string = std::format("[{}::{}] " FMT_STR " | {}", __log_folder, __log_file_name, __VA_ARGS__, std::source_location::current().function_name());                                                  \
        if      constexpr(LEVEL == ::bibstd::util::logger_level::debug)   { ::bibstd::util::log_debug(log_string); }                                                                                                    \
        else if constexpr(LEVEL == ::bibstd::util::logger_level::info)    { ::bibstd::util::log_info(log_string);  }                                                                                                    \
        else if constexpr(LEVEL == ::bibstd::util::logger_level::warning) { ::bibstd::util::log_warn(log_string);  }                                                                                                    \
        else if constexpr(LEVEL == ::bibstd::util::logger_level::error)   { ::bibstd::util::log_error(log_string); }                                                                                                    \
      }                                                                                                                                                                                                                 \
      catch(std::format_error& exception)                                                                                                                                                                               \
      {                                                                                                                                                                                                                 \
        ::bibstd::util::log_error(std::format("[INTERNAL_FMT_STR] format error: {}.", exception.what()));                                                                                                               \
      }                                                                                                                                                                                                                 \
    }                                                                                                                                                                                                                   \
  }
// clang-format on

///
/// Default logging macros
///
#define LOG_DEBUG(FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(::bibstd::util::logger_level::debug, FMT_STR, __VA_ARGS__);
#define LOG_INFO(FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(::bibstd::util::logger_level::info, FMT_STR, __VA_ARGS__);
#define LOG_WARN(FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(::bibstd::util::logger_level::warning, FMT_STR, __VA_ARGS__);
#define LOG_ERROR(FMT_STR, ...) INT_LOG_INTERNAL_FMT_STR(::bibstd::util::logger_level::error, FMT_STR, __VA_ARGS__);
