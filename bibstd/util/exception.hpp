#pragma once

#include "bibstd/util/log.hpp"

#include <source_location>
#include <stacktrace>
#include <stdexcept>
#include <string>

namespace bibstd::util
{

///
/// Exception base class.
///
class exception : public std::runtime_error
{
public: // Constructor and destructor
  inline exception(
    const std::string& error, const std::source_location& loc = std::source_location::current()
    /*, const std::stacktrace& stacktrace = std::stacktrace::current()*/
  );

public: // Accessors
  inline auto where() const -> const std::source_location& { return location_; }
  // auto stack() const -> const std::stacktrace& { return stacktrace_; }

protected: // Variables
  const std::source_location location_;
  // const std::stacktrace stacktrace_;
};

///
///
inline exception::exception(const std::string& error, const std::source_location& loc /*, const std::stacktrace& stacktrace*/)
  : std::runtime_error{error}
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
      std::format("{}\n  file: {}:{}:{}\n  function: {}", msg, loc.file_name(), loc.line(), loc.column(), loc.function_name()),
      ctx
    );
  }
};

///
/// Throw exception helper macro for throwing and logging.
///
#define THROW_EXCEPTION(WHAT)                                                                                                  \
  {                                                                                                                            \
    static_assert(std::convertible_to<std::decay_t<decltype(WHAT)>, std::string>);                                                  \
    constexpr auto loc = std::source_location::current();                                                                      \
    LOG_ERROR(                                                                                                                 \
      "exception",                                                                                                             \
      "{}\n  file: {}:{}:{}\n  function: {}",                                                                                  \
      WHAT,                                                                                                                    \
      loc.file_name(),                                                                                                         \
      loc.line(),                                                                                                              \
      loc.column(),                                                                                                            \
      loc.function_name()                                                                                                      \
    );                                                                                                                         \
    throw(::bibstd::util::exception(WHAT, loc));                                                                               \
  }
