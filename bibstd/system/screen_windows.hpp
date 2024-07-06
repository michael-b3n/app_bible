#pragma once

#include "system/screen_base.hpp"
#include "system/windows.hpp"
#include "util/bitmap.hpp"
#include "util/exception.hpp"
#include "util/log.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <ranges>

namespace bibstd::system
{

///
/// Screen capture class for windows implementation.
///
class screen final : public screen_base
{
public: // Static accessors
  ///
  /// Get the virtual screen metrics.
  /// \return screen metrics
  ///
  static inline auto metrics() -> screen_rect_type;

  ///
  /// Get the cursor position in virtual screen coordinate system.
  /// \return screen metrics
  ///
  static inline auto cursor_position() -> screen_coordinates_type;

  ///
  /// Capture screen in region defined by a rectangle.
  /// \param rect Rectangle of screen area that shall be captured
  /// \return captured pixels in screen capture bitmap object
  ///
  static inline auto capture(screen_rect_type rect) -> std::expected<util::bitmap, std::string>;

  ///
  /// Save image to `*.bmp` file.
  /// \param path File path for saving the image
  /// \param bmp Bitmap object
  ///
  static inline auto save(const std::filesystem::path& path, const util::bitmap& bmp) -> void;
};

///
///
inline auto screen::metrics() -> screen_rect_type
{
  // This should be set with a application manifest. This did not work
  // We set the Dpi awareness explicitly for this process.
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto x = GetSystemMetrics(SM_XVIRTUALSCREEN);
  const auto y = GetSystemMetrics(SM_YVIRTUALSCREEN);
  const auto width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  const auto height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  return screen_rect_type(screen_rect_type::coordinates_type(x, y), screen_rect_type::coordinates_type(x + width, y + height));
}

///
///
inline auto screen::cursor_position() -> screen_coordinates_type
{
  POINT point;
  // This should be set with a application manifest. This did not work
  // We set the Dpi awareness explicitly for this process.
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  if(!GetCursorPos(&point))
  {
    THROW_EXCEPTION(util::exception("Failed to get cursor position"));
  }
  return screen_coordinates_type(point.x, point.y);
}

///
///
inline auto screen::capture(const screen_rect_type rect) -> std::expected<util::bitmap, std::string>
{
  HDC hdc = GetDC(nullptr);
  HBITMAP bitmap = [&]
  {
    HDC sdc = CreateCompatibleDC(hdc);
    auto hbitmap = CreateCompatibleBitmap(hdc, rect.horizontal_range(), rect.vertical_range());
    HGDIOBJ hOld = SelectObject(sdc, hbitmap);
    const auto left_lower_coord = rect.left_lower_coordinates();
    BitBlt(
      sdc,
      0,
      0,
      rect.horizontal_range(),
      rect.vertical_range(),
      hdc,
      left_lower_coord.axis_value(0),
      left_lower_coord.axis_value(1),
      SRCCOPY | CAPTUREBLT);
    SelectObject(sdc, hOld);
    DeleteDC(sdc);
    return hbitmap;
  }();

  BITMAPINFO info = {0};
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  if(0 == GetDIBits(hdc, bitmap, 0, 0, nullptr, &info, DIB_RGB_COLORS))
  {
    return std::unexpected{"Bitmap info not found"};
  }
  info.bmiHeader.biCompression = BI_RGB;

  std::unique_ptr<std::byte[]> pixels_bytes(new std::byte[info.bmiHeader.biSizeImage]);
  if(0 == GetDIBits(hdc, bitmap, 0, info.bmiHeader.biHeight, static_cast<void*>(pixels_bytes.get()), &info, DIB_RGB_COLORS))
  {
    return std::unexpected{"Bitmap data not found"};
  }
  DeleteObject(bitmap);
  ReleaseDC(nullptr, hdc);

  assert(info.bmiHeader.biBitCount >= 24);
  const auto size = static_cast<std::size_t>(info.bmiHeader.biWidth * info.bmiHeader.biHeight);
  const auto byte_count = info.bmiHeader.biBitCount / 8;
  util::bitmap::data_type pixels(size);
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, size),
    [&](const auto index)
    {
      const auto i = index * byte_count;
      auto& pix = pixels.at(index);
      pix.blue = static_cast<std::uint8_t>(pixels_bytes.get()[i + 0]);
      pix.green = static_cast<std::uint8_t>(pixels_bytes.get()[i + 1]);
      pix.red = static_cast<std::uint8_t>(pixels_bytes.get()[i + 2]);
    });
  auto bmp = util::bitmap();
  bmp.data(info.bmiHeader.biWidth, info.bmiHeader.biHeight, std::move(pixels));
  return bmp;
}

///
///
inline auto screen::save(const std::filesystem::path& path, const util::bitmap& bmp) -> void
{
  bmp.save(path);
}

} // namespace bibstd::system
