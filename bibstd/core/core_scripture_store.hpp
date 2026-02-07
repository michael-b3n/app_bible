#pragma once

#include <memory>
#include <vector>

namespace bibstd::bible
{
// Forward declarations
class parser;
} // namespace bibstd::bible
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
  ~core_scripture_store() noexcept;

public: // Modifiers

private: // Implementation
  auto load_usx(const io::zip_file_reader& zip_reader) -> bool;

private: // Variables
  std::vector<std::unique_ptr<bible::parser>> scripture_data_;
};

} // namespace bibstd::core
