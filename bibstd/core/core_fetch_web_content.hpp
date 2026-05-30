#pragma once

#include <expected>
#include <string>
#include <string_view>

namespace bibstd::core
{

///
/// Core web content fetcher. This class uses libcurl to fetch content from a webpage.
///
class core_fetch_web_content final
{
public: // Typedefs
  ///
  /// Error codes for web content fetching operations.
  ///
  enum class error_code
  {
    init_failed,    ///< Failed to initialize curl
    request_failed, ///< HTTP request failed
    invalid_url,    ///< Invalid URL provided
    timeout,        ///< Request timed out
    unknown         ///< Unknown error occurred
  };

public: // Structors
  core_fetch_web_content() = default;
  ~core_fetch_web_content() noexcept = default;

public: // Operations
  ///
  /// Fetch content from a webpage.
  /// \param url The URL of the webpage to fetch
  /// \return Expected string containing the webpage content, or an error code
  ///
  auto fetch(std::string_view url) const -> std::expected<std::string, error_code>;
};

} // namespace bibstd::core
