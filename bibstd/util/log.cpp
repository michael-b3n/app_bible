#include "bibstd/util/log.hpp"
#include "bibstd/system/filesystem.hpp"
#include "bibstd/util/date.hpp"

#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/spdlog.h>

namespace bibstd::util
{
namespace
{

///
/// Constants
///
const auto logger_name = std::string{"main"};

///
/// Convert u8 string view to normal string.
/// \warning Normal strings are encoded in utf-8.
/// \param u8string that shall be converted
/// \return string with content of string
///
inline auto to_string(const std::u8string& u8string) -> std::string
{
  return std::string{u8string.begin(), u8string.end()};
}

///
/// Lock logger mutex.
///
inline auto lock_logger() -> std::scoped_lock<std::mutex>
{
  static std::mutex mtx;
  return std::scoped_lock{mtx};
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
  const auto log_filename = format_current_time_cet() + std::string{".log"};
  const auto log_file = log_directory_name / log_filename;
  const auto log_file_latest = log_directory_name / std::string{"latest.log"};

  const auto log_file_str = to_string(log_file.u8string());
  const auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_str, true);
  file_sink->set_level(spdlog::level::info);
  file_sink->set_pattern(std::string{log_pattern});

  const auto file_sink_latest =
    std::make_shared<spdlog::sinks::basic_file_sink_mt>(to_string(log_file_latest.u8string()), true);
  file_sink_latest->set_level(spdlog::level::debug);
  file_sink_latest->set_pattern(std::string{log_pattern});

  const auto sinks = std::vector<spdlog::sink_ptr>{file_sink, file_sink_latest};
  auto logger = std::make_shared<spdlog::logger>(logger_name, sinks.begin(), sinks.end());
  logger->set_level(spdlog::level::debug);
  logger->flush_on(spdlog::level::info);
  spdlog::register_logger(logger);

  log_debug(std::format("Init logger: file={}.", log_file_str));
}

///
/// Access logger.
///
inline auto get_logger() -> std::shared_ptr<spdlog::logger>
{
  return spdlog::get(logger_name);
}

} // namespace

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
  const auto lock = lock_logger();
  if(const auto logger = get_logger())
  {
    logger->debug(msg);
  }
}

///
///
auto log_info(std::string_view&& msg) -> void
{
  const auto lock = lock_logger();
  if(const auto logger = get_logger())
  {
    logger->info(msg);
  }
}

///
///
auto log_warn(std::string_view&& msg) -> void
{
  const auto lock = lock_logger();
  if(const auto logger = get_logger())
  {
    logger->warn(msg);
  }
}

///
///
auto log_error(std::string_view&& msg) -> void
{
  const auto lock = lock_logger();
  if(const auto logger = get_logger())
  {
    logger->error(msg);
  }
}

///
///
logger::logger()
{
  init_log();
}

///
///
logger::~logger() noexcept
{
  spdlog::shutdown();
}

} // namespace bibstd::util
