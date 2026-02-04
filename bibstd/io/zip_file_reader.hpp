#pragma once

#include "bibstd/util/non_owning_ptr.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <vector>

// Forward declarations for libzip types
struct zip;
struct zip_stat;

namespace bibstd::io
{

// Forward declarations
class zip_entry;

///
/// Reader for ZIP archives. Provides read-only access to ZIP file contents.
/// This class wraps libzip for simple ZIP file reading operations.
///
class zip_file_reader final
{
public: // Typedefs
  ///
  /// Compression methods supported by ZIP archives.
  ///
  enum class compression_method
  {
    default_method,
    store,   // No compression
    deflate, // Deflate compression
    bzip2,   // BZIP2 compression
    lzma,    // LZMA compression
    zstd,    // Zstandard compression
    xz       // XZ compression
  };

  ///
  /// Encryption methods supported by ZIP archives.
  ///
  enum class encryption_method
  {
    none,
    trad_pkware,
    aes_128,
    aes_192,
    aes_256
  };

  ///
  /// Flag of the archive to query.
  ///
  enum class query_flag
  {
    none,
    exclude_directories,
    exclude_directories_and_case_insensitive,
    case_insensitive
  };

  ///
  /// Represents an entry (file or directory) in a ZIP archive.
  ///
  struct zip_entry final
  {
    std::string comment;                           // entry comment
    std::optional<std::string> name;               // entry name (path within archive)
    std::optional<std::uint64_t> index;            // entry index in archive
    std::optional<compression_method> compression; // compression method
    std::optional<encryption_method> encryption;   // encryption method
    std::optional<std::uint64_t> size;             // uncompressed size
    std::optional<std::uint64_t> compressed_size;  // compressed size
    std::optional<std::uint32_t> crc;              // CRC32 checksum
  };

public: // Structors
  ///
  /// Construct a ZIP reader for the specified file path.
  /// \param zip_path Path to the ZIP archive
  /// \param password Optional password for encrypted archives
  ///
  zip_file_reader(const std::filesystem::path& zip_path, std::string_view password = "");

  ///
  /// Construct a ZIP reader from in-memory data.
  /// \param data Span of bytes containing the ZIP archive
  /// \param password Optional password for encrypted archives
  ///
  zip_file_reader(std::span<const std::byte> data, std::string_view password = "");

  ///
  /// Destructor. Closes the archive if open.
  ///
  ~zip_file_reader() noexcept;

  zip_file_reader(const zip_file_reader&) = delete;
  zip_file_reader(zip_file_reader&&) = default;

public: // Operators
  auto operator=(const zip_file_reader&) -> zip_file_reader& = delete;
  auto operator=(zip_file_reader&&) -> zip_file_reader& = default;

public: // Accessors
  ///
  /// Check if the archive is currently open.
  /// \return true if open, false otherwise
  ///
  [[nodiscard]] auto is_open() const -> bool;

  ///
  /// Get the number of entries in the archive.
  /// \return Number of entries
  ///
  [[nodiscard]] auto entry_count() const -> std::size_t;

  ///
  /// Get the archive comment.
  /// \warning Requires open archive.
  /// \return Archive comment, or empty string if none or error
  ///
  [[nodiscard]] auto comment() const -> std::string;

  ///
  /// Get all entries in the archive.
  /// \return Vector of all entries, empty if archive is not open
  ///
  [[nodiscard]] auto entries() const -> std::vector<zip_entry>;

  ///
  /// Get entry by index.
  /// \param index Entry index
  /// \return Entry if found, std::nullopt otherwise
  ///
  [[nodiscard]] auto entry(std::size_t index) const -> std::optional<zip_entry>;

  ///
  /// Get entry by name.
  /// \param name Entry name to find
  /// \param flags  Flags for querying the name
  /// \return Entry if found, null entry otherwise
  ///
  [[nodiscard]] auto entry(const std::string& name, query_flag flags = query_flag::none) const -> std::optional<zip_entry>;

  ///
  /// Check if an entry with the specified name exists.
  /// \param name Entry name to search for
  /// \param flags  Flags for querying the name
  /// \return true if entry exists, false otherwise
  ///
  [[nodiscard]] auto has_entry(const std::string& name, query_flag flags = query_flag::none) const -> bool;

  ///
  /// Read an entry's content into memory.
  /// \param entry Entry to read
  /// \return Binary data as vector, empty if error
  ///
  [[nodiscard]] auto read_entry(const zip_entry& entry) const -> std::vector<std::byte>;

  ///
  /// Read an entry's content as text string.
  /// \param entry Entry to read
  /// \return Text content as string, empty if error
  ///
  [[nodiscard]] auto read_entry_as_text(const zip_entry& entry) const -> std::string;

private: // Helpers
  ///
  /// Checks if archive contains entry with name.
  /// \param name Name to find
  /// \param flag Flags for querying the name
  /// \return true if entry is found, false otherwise
  ///
  [[nodiscard]] auto index_of_entry(const std::string& name, query_flag flag = query_flag::none) const
    -> std::optional<std::size_t>;

  ///
  /// Get entry by index.
  /// \param index Entry index
  /// \param flag Flags for zip_stat_index
  /// \return Entry if found, std::nullopt otherwise
  ///
  [[nodiscard]] auto entry_by_index(std::size_t index, std::uint32_t flag) const -> std::optional<zip_entry>;

  ///
  /// Create a zip_entry from libzip stat structure.
  /// \param stat Pointer to zip_stat structure
  /// \return Created zip_entry
  ///
  [[nodiscard]] auto create_entry(const zip_stat& stat) const -> zip_entry;

  ///
  /// Get last error message from libzip.
  /// \return Error message string
  ///
  [[nodiscard]] auto error_message() const -> std::string;

private: // Variables
  std::string password_;
  util::non_owning_ptr<zip> zip_handle_;
};

} // namespace bibstd::io
