#pragma once

#include "math/rect.hpp"

#include <cstdint>
#include <filesystem>
#include <ranges>
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
  static constexpr std::uint16_t bytes_per_pixel = bits_per_pixel / 8;

public: // Typedefs
  using rect_type = math::rect<std::uint32_t>;
  using coordinates_type = rect_type::coordinates_type;

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

public: // Accessors
  ///
  /// Checks if bitmap file is empty.
  /// \return boolean value of whether the bitmap is empty or not
  ///
  auto empty() const -> bool;

  ///
  /// Return the dimension of the bitmap as a rectangle.
  /// \return dimension of bitmap
  ///
  auto dimension() const -> rect_type;

  ///
  /// Get the pixels contained by given parameter `rect` of the bitmap.
  /// \param rect Rectangle defining area of the returned pixels
  /// \return list of pixels contained in area defined by `rect`
  ///
  auto pixels() const -> const data_type&;

public: // Modifiers
  ///
  /// Saves the current image, represented by the list of pixels, as a
  /// *.bmp file with the name provided by the parameter. File extension
  /// is not forced but should be `*.bmp`.
  /// \param path of the filename to be written as a bmp image
  ///
  auto save(const std::filesystem::path& path) const -> void;

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
  rect_type rect_{coordinates_type(0, 0), coordinates_type(0, 0)};
  data_type data_;
};

///
/// Create a new bitmap with pixels from source bitmap contained in given rectangle.
/// \param rect Rectangle defining area of the returned bitmap
/// \return new bitmap with pixels contained in area defined by `rect`
///
auto create_subarea_bitmap(const bitmap& bitmap_source, bitmap::rect_type rect) -> bitmap;

} // namespace bibstd::util
