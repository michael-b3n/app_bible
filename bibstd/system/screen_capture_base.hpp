#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace bibstd::system
{

///
/// Screen capture base class for all system implementations.
///
struct screen_capture_base
{
  // Typedefs
  ///
  /// Display metric in a struct.
  ///
  struct screen_metrics final
  {
    int x;
    int y;
    std::uint32_t width;
    std::uint32_t height;
  };

  ///
  /// Image class containing pixels and dimensions of image.
  ///
  class image final
  {
  public: // Constructor
    inline image(std::vector<std::uint8_t>&& pixels, std::uint32_t width, std::uint32_t height, std::uint16_t bits_per_pixel);

  public: // Variables
    const std::vector<std::uint8_t> pixels;
    const std::uint32_t width;
    const std::uint32_t height;
    const std::uint16_t bits_per_pixel;
    const std::uint16_t bytes_per_pixel;
  };

  // Constants
  static constexpr std::string_view log_channel = "screen_capture";
};

///
///
screen_capture_base::image::image(
  std::vector<std::uint8_t>&& pixels_, const std::uint32_t width_, const std::uint32_t height_, const std::uint16_t bits_per_pixel_)
  : pixels{std::forward<decltype(pixels_)>(pixels_)}
  , width{width_}
  , height{height_}
  , bits_per_pixel{bits_per_pixel_}
  , bytes_per_pixel{static_cast<std::uint16_t>(bits_per_pixel_ / 8)}
{
}

} // namespace bibstd::system
