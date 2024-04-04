#pragma once

#include "util/log.hpp"

#include <source_location>
#include <stacktrace>
#include <stdexcept>
#include <string>

namespace bibstd::util
{

///
/// Exception base class.
///
class exception : public std::exception
{
public: // Constructor and destructor
  inline exception(
    std::string error, const std::source_location& loc = std::source_location::current()
    /*, const std::stacktrace& stacktrace = std::stacktrace::current()*/);

public: // Accessors
  inline auto what() const noexcept -> const char* { return error_.data(); }
  inline auto where() const -> const std::source_location& { return location_; }
  // auto stack() const -> const std::stacktrace& { return stacktrace_; }

protected: // Variables
  const std::string error_;
  const std::source_location location_;
  // const std::stacktrace stacktrace_;
};

///
///
inline exception::exception(std::string error, const std::source_location& loc /*, const std::stacktrace& stacktrace*/)
  : error_{std::forward<std::string>(error)}
  , location_{loc} // , stacktrace_{stacktrace}
{
}

} // namespace bibstd::util

///
///
template<>
struct std::formatter<bibstd::util::exception> : std::formatter<std::string>
{
  auto format(const bibstd::util::exception& ex, std::format_context& ctx) const
  {
    decltype(auto) msg = ex.what();
    decltype(auto) loc = ex.where();
    return formatter<std::string>::format(
      std::format("{}\n  file: {}:{}:{}\n  function: {}", msg, loc.file_name(), loc.line(), loc.column(), loc.function_name()), ctx);
  }
};

///
/// Throw exception helper macro for throwing and logging.
///
#define THROW_EXCEPTION(EX)                                                                                                                          \
  {                                                                                                                                                  \
    static_assert(std::is_base_of_v<::bibstd::util::exception, decltype(EX)>);                                                                       \
    LOG_ERROR("exception", "{}", EX);                                                                                                                \
    throw(EX);                                                                                                                                       \
  }
