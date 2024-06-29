#include "util/bitmap.hpp"
#include "math/arithmetic.hpp"
#include "util/log.hpp"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <ranges>

namespace bibstd::util
{

///
/// Windows BMP-specific format data
///
struct bmp_file_magic final
{
  static constexpr std::size_t size = 2;
  unsigned char magic[size];
};

struct bmp_file_header final
{
  std::uint32_t file_size;
  std::uint16_t creator1;
  std::uint16_t creator2;
  std::uint32_t bmp_offset;
};

struct bmp_file_dib_info final
{
  std::uint32_t header_size;
  std::int32_t width;
  std::int32_t height;
  std::uint16_t num_planes;
  std::uint16_t bits_per_pixel;
  std::uint32_t compression;
  std::uint32_t bmp_byte_size;
  std::int32_t hres;
  std::int32_t vres;
  std::uint32_t num_colors;
  std::uint32_t num_important_colors;
};

///
///
auto bitmap::save(const std::filesystem::path& path) const -> void
{
  std::ofstream file(path, std::ios::out | std::ios::binary);
  if(file.fail())
  {
    LOG_ERROR(log_channel, "File could not be opened for editing: {}", path.string());
  }
  else if(empty())
  {
    LOG_WARN(log_channel, "Empty bitmap not be saved to {}", path.string());
  }
  else
  {
    // Calculate row and bitmap size
    const std::int32_t row_size = width_ * 3 + width_ % 4; // The size of each row is rounded up to a multiple of 4 bytes by padding.
    const std::uint32_t bitmap_size = row_size * height_;

    // Write all the header information that the BMP file format requires.
    bmp_file_magic magic;
    magic.magic[0] = 'B';
    magic.magic[1] = 'M';
    file.write((char*)(&magic), sizeof(magic));
    bmp_file_header header = {0};
    header.bmp_offset = sizeof(bmp_file_magic) + sizeof(bmp_file_header) + sizeof(bmp_file_dib_info);
    header.file_size = header.bmp_offset + bitmap_size;
    file.write((char*)(&header), sizeof(header));
    bmp_file_dib_info dib_info = {0};
    dib_info.header_size = sizeof(bmp_file_dib_info);
    dib_info.width = width_;
    dib_info.height = height_;
    dib_info.num_planes = 1;
    dib_info.bits_per_pixel = bits_per_pixel;
    dib_info.compression = 0;
    dib_info.bmp_byte_size = 0;
    dib_info.hres = 2835;
    dib_info.vres = 2835;
    dib_info.num_colors = 0;
    dib_info.num_important_colors = 0;
    file.write((char*)(&dib_info), sizeof(dib_info));

    // Write each row and column of Pixels into the image file -- we write
    // the rows upside-down to satisfy the easiest BMP format.
    std::ranges::for_each(
      std::views::iota(std::uint32_t{0}, height_),
      [&](const auto row_idx)
      {
        std::ranges::for_each(
          std::views::iota(width_ * row_idx, width_ * (row_idx + 1)),
          [&](const auto index)
          {
            const auto& pix = data_.at(index);
            file.put(static_cast<unsigned char>(pix.blue));
            file.put(static_cast<unsigned char>(pix.green));
            file.put(static_cast<unsigned char>(pix.red));
          });
        if(const auto pad = width_ % 4; pad != 0)
        {
          // Rows are padded so that they're always a multiple of 4
          // bytes. This line skips the padding at the end of each row.
          std::ranges::for_each(std::views::iota(std::size_t{0}) | std::ranges::views::take(pad), [&]([[maybe_unused]] auto) { file.put(0); });
        }
      });
    file.close();
  }
}

///
///
auto bitmap::empty() const -> bool
{
  return data_.empty();
}

///
///
auto bitmap::data(const std::uint32_t width, const std::uint32_t height, data_type&& data) -> void
{
  if(const auto size = math::arithmetic::multiply(width, height); !size.has_value() || size.value() != data.size())
  {
    THROW_EXCEPTION(util::exception("Invalid data size"));
  }
  width_ = width;
  height_ = height;
  data_ = std::move(data);
}

} // namespace bibstd::util
