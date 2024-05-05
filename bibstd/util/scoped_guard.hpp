#pragma once

#include "util/log.hpp"

#include <functional>

namespace bibstd::util
{

///
/// Guard class calling callable on destruction.
///
class scoped_guard final
{
public: // Constructors
  inline scoped_guard(std::move_only_function<void()>&& on_destruction);
  inline ~scoped_guard() noexcept;
  inline scoped_guard(scoped_guard&& rhs) noexcept;
  inline scoped_guard(const scoped_guard&) = delete;

public: // Operators
  auto operator=(scoped_guard&& rhs) & noexcept -> scoped_guard&;
  auto operator=(const scoped_guard&) -> scoped_guard& = delete;

private:
  std::move_only_function<void()> on_destruction_;
};

///
///
inline scoped_guard::scoped_guard(std::move_only_function<void()>&& on_destruction)
  : on_destruction_{std::forward<decltype(on_destruction)>(on_destruction)}
{
}

///
///
inline scoped_guard::~scoped_guard() noexcept
{
  static constexpr std::string_view log_channel = "scoped_guard";
  try
  {
    if(on_destruction_)
    {
      on_destruction_();
    }
  }
  catch(const std::exception& e)
  {
    LOG_ERROR(log_channel, "Scoped guard destruction exception: {}", e.what());
  }
  catch(...)
  {
    LOG_ERROR(log_channel, "Scoped guard destruction exception: {}", "unknown exception");
  }
}

///
///
inline scoped_guard::scoped_guard(scoped_guard&& rhs) noexcept
  : on_destruction_{std::move(rhs.on_destruction_)}
{
  rhs.on_destruction_ = nullptr;
}

///
///
auto scoped_guard::operator=(scoped_guard&& rhs) & noexcept -> scoped_guard&
{
  on_destruction_ = std::move(rhs.on_destruction_);
  rhs.on_destruction_ = nullptr;
  return *this;
}

} // namespace bibstd::util
