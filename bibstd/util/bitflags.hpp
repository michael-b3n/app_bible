#pragma once

#include "bibstd/meta/type_traits.hpp"
#include "bibstd/util/enum.hpp"

#include "magic_enum/magic_enum.hpp"

#include <algorithm>
#include <bitset>

namespace bibstd::util
{

///
/// Class for managing bitflags based on enum values.
/// This class allows combining multiple enum values into
/// a single bitfield for efficient storage and querying.
///
template<enum_type E>
class bitflags final
{
public: // Constructor
  constexpr bitflags() = default;
  template<typename... Flags>
    requires(meta::are_same_v<E, Flags...>)
  constexpr bitflags(Flags... flags);

public: // Modifiers
  ///
  /// Set a flag to true.
  /// \param flag The flag to set
  ///
  constexpr auto set(E flag) -> void;

  ///
  /// Set a flag to false.
  /// \param flag The flag to reset
  ///
  constexpr auto reset(E flag) -> void;

  ///
  /// Reset all flags to false.
  ///
  constexpr auto reset() -> void;

  ///
  /// Flip a flag.
  /// \param flag The flag to flip
  /// \return Reference to this
  ///
  constexpr auto flip(E flag) -> bitflags&;

  ///
  /// Flip all flags.
  /// \return Reference to this
  ///
  constexpr auto flip() -> bitflags&;

public: // Accessors
  ///
  /// Test if a flag is set.
  /// \param flag The flag to test
  /// \return true if the flag is set, false otherwise
  ///
  constexpr auto test(E flag) const -> bool;

  ///
  /// Check if a flag is set (alias for test).
  /// \param flag The flag to check
  /// \return true if the flag is set, false otherwise
  ///
  constexpr auto has(E flag) const -> bool;

  ///
  /// Check if any of the flags in other are set in this.
  /// \param other The flags to check
  /// \return true if any flags match, false otherwise
  ///
  constexpr auto has_any(bitflags other) const -> bool;

  ///
  /// Check if all of the flags in other are set in this.
  /// \param other The flags to check
  /// \return true if all flags match, false otherwise
  ///
  constexpr auto has_all(bitflags other) const -> bool;

  ///
  /// Check if all bits are set to true.
  /// \return true if all bits are set, false otherwise
  ///
  constexpr auto all() const -> bool;

  ///
  /// Check if any bit is set to true.
  /// \return true if any bit is set, false otherwise
  ///
  constexpr auto any() const -> bool;

  ///
  /// Check if no bits are set to true.
  /// \return true if no bits are set, false otherwise
  ///
  constexpr auto none() const -> bool;

  ///
  /// Count the number of bits set to true.
  /// \return Number of bits set to true
  ///
  constexpr auto count() const -> std::size_t;

  ///
  /// Get the number of bits that the bitflags holds.
  /// \return Total number of bits
  ///
  constexpr auto size() const -> std::size_t;

public: // Operators
  constexpr auto operator==(const bitflags&) const -> bool = default;
  constexpr auto operator|=(bitflags other) -> bitflags&;
  constexpr auto operator|=(E flag) -> bitflags&;
  constexpr auto operator&=(bitflags other) -> bitflags&;
  constexpr auto operator&=(E flag) -> bitflags&;
  constexpr auto operator^=(bitflags other) -> bitflags&;
  constexpr auto operator^=(E flag) -> bitflags&;
  constexpr auto operator~() const -> bitflags;

private: // Constants
  // clang-format off
  static_assert(
    magic_enum::enum_count<E>() > 0,
    "requires at least one enum value");
  static_assert(
    [] { return std::ranges::all_of(magic_enum::enum_values<E>(), [c = 0](const auto v) mutable { return to_integral(v) == c++; }); }(),
    "requires that the enum values are sequential starting from 0");
  // clang-format on

private: // Typedefs
  using bitset_type = std::bitset<magic_enum::enum_count<E>()>;

private: // Constructors
  constexpr explicit bitflags(bitset_type bits);

private: // Variables
  bitset_type flags_{};
};

///
///
template<enum_type E>
template<typename... Flags>
  requires(meta::are_same_v<E, Flags...>)
constexpr bitflags<E>::bitflags(Flags... flags)
{
  (flags_.set(to_integral(flags)), ...);
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::set(const E flag) -> void
{
  flags_.set(to_integral(flag));
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::reset(const E flag) -> void
{
  flags_.reset(to_integral(flag));
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::reset() -> void
{
  flags_.reset();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::flip(const E flag) -> bitflags&
{
  flags_.flip(to_integral(flag));
  return *this;
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::flip() -> bitflags&
{
  flags_.flip();
  return *this;
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::test(const E flag) const -> bool
{
  return flags_.test(to_integral(flag));
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::has(const E flag) const -> bool
{
  return test(flag);
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::has_any(const bitflags other) const -> bool
{
  return (flags_ & other.flags_).any();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::has_all(const bitflags other) const -> bool
{
  return (flags_ & other.flags_) == other.flags_;
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::all() const -> bool
{
  return flags_.all();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::any() const -> bool
{
  return flags_.any();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::none() const -> bool
{
  return flags_.none();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::count() const -> std::size_t
{
  return flags_.count();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::size() const -> std::size_t
{
  return magic_enum::enum_count<E>();
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator|=(const bitflags other) -> bitflags&
{
  flags_ |= other.flags_;
  return *this;
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator|=(const E flag) -> bitflags&
{
  return *this |= bitflags{flag};
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator&=(const bitflags other) -> bitflags&
{
  flags_ &= other.flags_;
  return *this;
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator&=(const E flag) -> bitflags&
{
  return *this &= bitflags{flag};
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator^=(const bitflags other) -> bitflags&
{
  flags_ ^= other.flags_;
  return *this;
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator^=(const E flag) -> bitflags&
{
  return *this ^= bitflags{flag};
}

///
///
template<enum_type E>
constexpr auto bitflags<E>::operator~() const -> bitflags
{
  return bitflags{~flags_};
}

///
///
template<enum_type E>
constexpr bitflags<E>::bitflags(const bitset_type bits)
  : flags_{bits}
{
}

} // namespace bibstd::util
