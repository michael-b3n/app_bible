#include "util/log.hpp"
#include "system/filesystem.hpp"
#include "util/date.hpp"
#include "util/string.hpp"
#include "util/string.hpp"

#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace bibstd::util
{
namespace detail
{

///
/// Constants
///
static const auto logger_name = std::string{"main"};

///
/// Lock logger mutex.
///
inline auto lock_logger() -> std::lock_guard<std::mutex>
{
  static std::mutex mtx;
  return std::lock_guard(mtx);
}

///
/// Setup logger.
///
inline auto init_log() -> void
{
  static constexpr auto log_dir = std::string_view("logs");
  static constexpr auto log_pattern = std::string_view("[%Y-%m-%d %H:%M:%S] [%L] [%t] %v");

  const auto local_data_path = system::filesystem::local_data_folder();
  std::filesystem::create_directories(local_data_path / log_dir);
  const auto log_directory_name = local_data_path / log_dir;
  const auto log_filename = format_current_time_CET() + std::string{".log"};
  const auto log_file = log_directory_name / log_filename;
  const auto log_file_latest = log_directory_name / std::string{"latest.log"};

  const auto log_file_str = to_string(log_file.u8string());
  const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_str, true);
  file_sink->set_level(spdlog::level::info);
  file_sink->set_pattern(util::to_string(log_pattern));

  const auto file_sink_latest = std::make_shared<spdlog::sinks::basic_file_sink_mt>(to_string(log_file_latest.u8string()), true);
  file_sink_latest->set_level(spdlog::level::debug);
  file_sink_latest->set_pattern(util::to_string(log_pattern));

  const auto sinks = std::vector<spdlog::sink_ptr>{file_sink, file_sink_latest};
  auto logger = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());
  logger->set_level(spdlog::level::debug);
  logger->flush_on(spdlog::level::warn);
  spdlog::register_logger(logger);

  log_debug(std::format("Init logger: file={}.", log_file_str));
}

///
/// Access logger.
///
inline auto logger() -> std::shared_ptr<spdlog::logger>
{
  return spdlog::get(logger_name);
}

} // namespace detail

///
///
auto global_log_level() -> logger_level
{
  return logger_level::debug;
}

///
///
auto log_debug(std::string_view&& msg) -> void
{
  const auto lock = detail::lock_logger();
  detail::logger()->debug(std::move(msg));
}

///
///
auto log_info(std::string_view&& msg) -> void
{
  const auto lock = detail::lock_logger();
  detail::logger()->info(std::move(msg));
}

///
///
auto log_warn(std::string_view&& msg) -> void
{
  const auto lock = detail::lock_logger();
  detail::logger()->warn(std::move(msg));
}

///
///
auto log_error(std::string_view&& msg) -> void
{
  const auto lock = detail::lock_logger();
  detail::logger()->error(msg);
}

///
///
logger::logger()
{
  detail::init_log();
}

///
///
logger::~logger() noexcept
{
  spdlog::shutdown();
}

} // namespace bibstd::util
