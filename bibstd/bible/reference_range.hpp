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
  reference_range(reference first_and_last);
  reference_range(reference first, reference second);

public: // Accessors
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
