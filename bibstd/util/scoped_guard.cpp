#include "util/scoped_guard.hpp"
#include "util/log.hpp"

namespace bibstd::util
{

///
///
scoped_guard::scoped_guard(std::move_only_function<void()>&& on_destruction)
  : on_destruction_{std::forward<decltype(on_destruction)>(on_destruction)}
{
}

///
///
scoped_guard::~scoped_guard() noexcept
{
  destruct();
}

///
///
scoped_guard::scoped_guard(scoped_guard&& rhs) noexcept
  : on_destruction_{std::move(rhs.on_destruction_)}
{
  rhs.on_destruction_ = nullptr;
}

///
///
auto scoped_guard::operator=(scoped_guard&& rhs) & noexcept -> scoped_guard&
{
  destruct();
  on_destruction_ = std::move(rhs.on_destruction_);
  rhs.on_destruction_ = nullptr;
  return *this;
}

///
///
auto scoped_guard::destruct() -> void
{
  static constexpr std::string_view log_channel = "scoped_guard";
  try
  {
    if(on_destruction_)
    {
      on_destruction_();
      on_destruction_ = nullptr;
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

} // namespace bibstd::util
