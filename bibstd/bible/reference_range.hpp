#pragma once

#include "bible/reference.hpp"

namespace bibstd::bible
{

///
/// Bible reference span class. This class contains a range of bible references.
///
class reference_range final
{
public: // Typedefs
  using chapter_type = reference::chapter_type;
  using verse_type = reference::verse_type;

public: // Constructor
  explicit reference_range(reference first_and_last);
  reference_range(reference first, reference second);

public: // Operators
  auto operator==(const reference_range&) const -> bool = default;

public: // Accessors
  ///
  /// Get the size of all references in the range.
  ///
  auto size() const -> std::uint32_t;

  ///
  /// Get the first reference in the range.
  /// \return the first reference.
  ///
  auto begin() const -> reference;

  ///
  /// Get the last reference in the range.
  /// \return the last reference.
  ///
  auto end() const -> reference;

private: // Variables
  reference from_;
  reference to_;
};

} // namespace bibstd::bible

///
///
template<>
struct std::formatter<bibstd::bible::reference_range> : std::formatter<bibstd::bible::reference>
{
  auto format(const bibstd::bible::reference_range e, std::format_context& ctx) const
  {
    return formatter<std::string>::format(std::format("{} - {}", e.begin(), e.end()), ctx);
  }
};
