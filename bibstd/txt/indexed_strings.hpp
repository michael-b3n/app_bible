#pragma once

#include <functional>
#include <string>
#include <vector>

namespace bibstd::txt
{

///
/// Index string containing a string and indices for each character.
///
class indexed_strings final
{
public: // Typedefs
  using data_type = std::vector<std::pair<std::size_t, std::string>>;
  using const_iterator = data_type::const_iterator;
  using const_reverse_iterator = data_type::const_reverse_iterator;

public: // Constructor
  indexed_strings() = default;

public: // Operators
  auto operator<=>(const indexed_strings&) const = default;

public: // Accessors
  ///
  /// Get the number of stored strings.
  /// \return number of stored strings
  ///
  auto size() const -> std::size_t;

  ///
  /// Get the string at the given index.
  /// \param index Index of the character
  /// \return string having the given index
  ///
  auto read_at(std::size_t index) const -> std::string;

  ///
  /// Get the joined string of all stored strings.
  /// \return joined string
  ///
  auto joined_string() const -> std::string;

  ///
  /// Get a list of all chars with their corresponding indexes within the index string object.
  /// \return vector of pairs containing the index and the character
  ///
  auto indexed_chars() const -> std::vector<std::pair<std::size_t, char>>;

public: // Modifiers
  ///
  /// Set the string at the given index.
  /// \param index Index of the string
  /// \param string String that shall be set
  ///
  auto overwrite_at(std::size_t index, std::string_view string) -> void;

  ///
  /// Append a string to the container with the corresponding index.
  /// \param string String that shall be appended
  ///
  auto append_string(std::string_view string) -> void;

private: // Variables
  std::vector<std::pair<std::size_t, std::string>> data_;
};

} // namespace bibstd::txt
