#include "data/save_as_bitmap.hpp"
#include "util/log.hpp"

#include <algorithm>
#include <fstream>
#include <ranges>

namespace bibstd::data
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
auto save_as_bitmap(const plane<pixel>& data, const std::filesystem::path& path) -> bool
{
  static constexpr std::string_view log_channel = "save_as_bitmap";
  if(path.extension() != std::string_view(".bmp"))
  {
    LOG_ERROR(log_channel, "Invalid file extension: {}", path.extension().string());
    return false;
  }
  std::ofstream file(path, std::ios::out | std::ios::binary);
  if(file.fail())
  {
    LOG_ERROR(log_channel, "File could not be opened for editing: {}", path.string());
    return false;
  }
  if(data.data.empty())
  {
    LOG_INFO(log_channel, "Empty pixels not be saved to {}", path.string());
    return true;
  }

  const auto width = data.width;
  const auto height = data.height;
  // Calculate row and pixels size
  const std::uint32_t row_size =
    width * 3 + width % 4; // The size of each row is rounded up to a multiple of 4 bytes by padding.
  const std::uint32_t bitmap_size = row_size * height;

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
  dib_info.width = width;
  dib_info.height = height;
  dib_info.num_planes = 1;
  dib_info.bits_per_pixel = 24;
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
    std::views::iota(std::uint32_t{0}, height) | std::views::reverse,
    [&](const auto row_idx)
    {
      std::ranges::for_each(
        std::views::iota(width * row_idx, width * (row_idx + 1)),
        [&](const auto index)
        {
          const auto& p = data.data.at(index);
          file.put(static_cast<unsigned char>(p.blue));
          file.put(static_cast<unsigned char>(p.green));
          file.put(static_cast<unsigned char>(p.red));
        }
      );
      if(const auto pad = width % 4; pad != 0)
      {
        // Rows are padded so that they're always a multiple of 4
        // bytes. This line skips the padding at the end of each row.
        std::ranges::for_each(
          std::views::iota(std::size_t{0}) | std::ranges::views::take(pad), [&]([[maybe_unused]] auto) { file.put(0); }
        );
      }
    }
  );
  file.close();
  return true;
}

} // namespace bibstd::data
