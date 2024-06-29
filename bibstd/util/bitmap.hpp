#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace bibstd::util
{

///
/// Represents a bitmap where a vector of pixels describes the color of each pixel within the image.
/// Limited to *.bmp formatted images with no compression and 24 bit color depth.
///
class bitmap final
{
public: // Constants
  static constexpr std::uint16_t bits_per_pixel = 24;

public: // Typedefs
  ///
  /// Represents a single 24 bit pixel in the image. A pixel has red, green and blue
  /// components that are mixed to form a color. Each of these values can range from 0 to 255.
  ///
  struct pixel final
  {
    // Constants
    static constexpr std::size_t bit_size = 24;

    // Variables
    std::uint8_t blue{0};
    std::uint8_t green{0};
    std::uint8_t red{0};
  };
  using data_type = std::vector<pixel>;

public: // Constructors
  bitmap() = default;

public: // Modifiers
  ///
  /// Saves the current image, represented by the list of pixels, as a
  /// *.bmp file with the name provided by the parameter. File extension
  /// is not forced but should be `*.bmp`.
  /// \param path of the filename to be written as a bmp image
  ///
  auto save(const std::filesystem::path& path) const -> void;

  ///
  /// Checks if bitmap file is empty.
  /// \return boolean value of whether the bitmap is empty or not
  ///
  auto empty() const -> bool;

  ///
  /// Overwrites the current bitmap with that represented by a list of pixels.
  /// \param width Width of the bitmap
  /// \param height Height of the bitmap
  /// \param data vector of pixels
  ///
  auto data(std::uint32_t width, std::uint32_t height, data_type&& data) -> void;

private: // Constants
  static constexpr std::string_view log_channel = "bitmap";

private: // Variables
  std::uint32_t width_{0};
  std::uint32_t height_{0};
  data_type data_;
};

} // namespace bibstd::util
