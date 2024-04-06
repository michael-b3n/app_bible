#pragma once

namespace bibstd::util
{

///
/// Non copyable utility class.
/// Inherit from this class to prevent copying derived class.
///
class non_copyable
{
protected: // Structors and operators
  ///
  /// Default constructor.
  ///
  constexpr non_copyable() noexcept = default;

  ///
  /// Default destructor.
  ///
  virtual ~non_copyable() noexcept = default;

  ///
  /// Deleted copy constructor.
  ///
  non_copyable(const non_copyable&) = delete;

  ///
  /// Default move constructor.
  ///
  non_copyable(non_copyable&&) noexcept = default;

  ///
  /// Deleted copy operator.
  ///
  non_copyable& operator=(const non_copyable&) = delete;

  ///
  /// Default move operator,
  ///
  non_copyable& operator=(non_copyable&&) noexcept = default;
};

} // namespace bibstd::util
