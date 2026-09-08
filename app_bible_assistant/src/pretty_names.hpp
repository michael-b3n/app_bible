#pragma once

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace aba
{

///
/// Table holding the pretty names of the application in all available languages.
/// The table is read from a CSV document with the following layout:
/// \code
///   key,<language>,<language>,...
///   <key>,<pretty name>,<pretty name>,...
/// \endcode
/// The first column holds the keys, all following columns hold the pretty names of one
/// language each. The name of a language column is the language identifier used for lookups.
/// Lines starting with '#' and empty lines are ignored.
///
/// Keys are the identifiers used by the backend, e.g. the path of a setting. The pretty name
/// of a setting value is stored under the key "<setting path>/<setting value>".
///
class pretty_names final
{
  // Variables
  std::vector<std::string> languages_;
  std::unordered_map<std::string, std::vector<std::string>> entries_;

public: // Structors
  pretty_names() = default;

  ///
  /// Construct the table from a CSV document.
  /// \throws util::exception if the document cannot be parsed
  ///
  explicit pretty_names(std::span<const std::byte> csv);

public: // Accessors
  ///
  /// Access all available languages in the order defined by the document.
  /// \return list of language identifiers
  ///
  [[nodiscard]] auto languages() const -> const std::vector<std::string>&;

  ///
  /// Access the pretty name of a key.
  /// \return pretty name, std::nullopt if the language or the key is unknown
  ///
  [[nodiscard]] auto name(std::string_view language, std::string_view key) const -> std::optional<std::string>;
};

} // namespace aba
