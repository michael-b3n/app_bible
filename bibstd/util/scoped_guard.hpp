#pragma once

#include <functional>

namespace bibstd::util
{

///
/// Guard class calling callable on destruction.
///
class scoped_guard final
{
public: // Constructors
  scoped_guard() = default;
  scoped_guard(std::move_only_function<void()>&& on_destruction);
  ~scoped_guard() noexcept;
  scoped_guard(scoped_guard&& rhs) noexcept;
  scoped_guard(const scoped_guard&) = delete;

public: // Operators
  auto operator=(scoped_guard&& rhs) & noexcept -> scoped_guard&;
  auto operator=(const scoped_guard&) -> scoped_guard& = delete;

private: // Implementation
  auto destruct() -> void;

private:
  std::move_only_function<void()> on_destruction_;
};

} // namespace bibstd::util
