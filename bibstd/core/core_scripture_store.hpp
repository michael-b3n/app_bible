#pragma once

namespace bibstd::io
{
// Forward declarations
class zip_file_reader;
} // namespace bibstd::io
namespace bibstd::core
{

///
/// Core scripture store. This class contains loaded scripture data.
///
class core_scripture_store final
{
public: // Typedefs
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
  core_scripture_store();
  ~core_scripture_store() noexcept = default;

public: // Modifiers

private: // Implementation
  auto load_usx(const io::zip_file_reader& zip_reader) const -> bool;

private: // Variables
};

} // namespace bibstd::core
