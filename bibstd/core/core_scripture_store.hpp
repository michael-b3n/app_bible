#pragma once

#include "bibstd/bible/scripture.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <string>

// Forward declarations
namespace bibstd::bible
{
class scripture;
} // namespace bibstd::bible
namespace bibstd::io
{
class zip_file_reader;
} // namespace bibstd::io

namespace bibstd::core
{

///
/// Core scripture store. This class contains the scripture data loaded from the files of a folder.
///
class core_scripture_store final
{
  // Variables
  std::map<std::string, std::shared_ptr<bible::scripture>> scripture_data_;

public: // Typedefs
  using scripture_map_type = decltype(scripture_data_);

  ///
  /// Supported file types for scripture data.
  ///
  enum class supported_file_type
  {
    zip,
  };

  ///
  /// Supported scripture formats.
  ///
  enum class supported_format_type
  {
    usx,
  };

public: // Structors
  ///
  /// Load every supported file directly inside \p folder, in the order of the file names.
  /// A file that fails to load is logged and left out.
  ///
  explicit core_scripture_store(const std::filesystem::path& folder);
  ~core_scripture_store() noexcept;

public: // Accessors
  ///
  /// \return Map of all loaded scriptures
  ///
  auto scriptures() const -> const scripture_map_type&;

private: // Implementation
  auto load_usx(const io::zip_file_reader& zip_reader) -> bool;
};

} // namespace bibstd::core
