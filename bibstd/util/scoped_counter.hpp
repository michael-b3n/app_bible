#pragma once

#include "util/non_owning_ptr.hpp"

#include <atomic>

namespace bibstd::util
{
// Forward declaration
class scoped_counter;

///
/// Scoped incrementor type handling decrementing count on destruction.
/// This class is created using `scoped_increment` in `scoped_counter`.
///
class scoped_incrementor final
{
public: // Structors
  scoped_incrementor();
  scoped_incrementor(non_owning_ptr<scoped_counter> parent);
  ~scoped_incrementor() noexcept;
  scoped_incrementor(const scoped_incrementor& other);
  scoped_incrementor(scoped_incrementor&& other) noexcept;

public: // Operators
  auto operator=(const scoped_incrementor& other) & -> scoped_incrementor&;
  auto operator=(scoped_incrementor&& other) & noexcept -> scoped_incrementor&;

private: // Variables
  non_owning_ptr<scoped_counter> parent_;
};

///
/// Scoped counter. Increment by getting a scoped incrementor element.
/// Decrement counter by releasing element.
///
class scoped_counter final
{
public: // Structors
  scoped_counter();

public: // Accessors
  ///
  /// Get current count.
  /// \return current number of counter
  ///
  auto count() const -> std::size_t;

public: // Modifiers
  ///
  /// Increment count and get scoped incrementor object.
  /// Count will be decremented on `scoped_incrementor` destruction.
  /// \return scoped incrementor
  ///
  auto scoped_increment() -> scoped_incrementor;

private: // Friends
  friend class scoped_incrementor;

private: // Variables
  std::atomic_uint64_t counter_;
};

} // namespace bibstd::util
