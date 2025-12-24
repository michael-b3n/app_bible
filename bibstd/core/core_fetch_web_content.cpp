#include "bibstd/core/core_fetch_web_content.hpp"
#include "bibstd/util/log.hpp"

#include <cstring>
#include <curl/curl.h>

namespace bibstd::core
{
namespace
{

///
/// Callback function for libcurl to write received data.
///
auto write_callback(char* ptr, std::size_t size, std::size_t nmemb, void* userdata) -> std::size_t
{
  const auto total_size = size * nmemb;
  auto* str = static_cast<std::string*>(userdata);
  str->append(ptr, total_size);
  return total_size;
}

} // namespace

///
///
auto core_fetch_web_content::fetch(const std::string_view url) const -> std::expected<std::string, error_code>
{
  if(url.empty())
  {
    LOG_ERROR("invalid empty url provided");
    return std::unexpected(error_code::invalid_url);
  }

  // Initialize CURL
  CURL* curl = curl_easy_init();
  if(!curl)
  {
    LOG_ERROR("failed to initialize curl");
    return std::unexpected(error_code::init_failed);
  }

  // Prepare result string
  std::string result;
  result.reserve(4096); // Reserve some space to reduce allocations

  // Set curl options
  const auto url_str = std::string(url);
  curl_easy_setopt(curl, CURLOPT_URL, url_str.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);       // 30 second timeout
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "bibstd-core-fetch/1.0");

  // Perform the request
  const CURLcode res = curl_easy_perform(curl);

  // Check for errors
  if(res != CURLE_OK)
  {
    const auto error_msg = curl_easy_strerror(res);
    LOG_ERROR("fetch url content failed on curl perform: url=\"{}\", error=\"{}\"", url, error_msg);

    // Cleanup before returning error
    curl_easy_cleanup(curl);

    // Map curl error to our error code
    switch(res)
    {
    case CURLE_URL_MALFORMAT: return std::unexpected(error_code::invalid_url);
    case CURLE_OPERATION_TIMEDOUT: return std::unexpected(error_code::timeout);
    default: return std::unexpected(error_code::request_failed);
    }
  }

  // Check HTTP response code
  long http_code = 0;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
  if(http_code >= 400)
  {
    LOG_ERROR("fetch url content failed on http request: url=\"{}\", http_error_code={}", url, http_code);
    curl_easy_cleanup(curl);
    return std::unexpected(error_code::request_failed);
  }

  // Cleanup
  curl_easy_cleanup(curl);

  LOG_DEBUG("fetch url content succeeded: url=\"{}\", bytes_fetched={}", url, result.size());
  return result;
}

} // namespace bibstd::core
