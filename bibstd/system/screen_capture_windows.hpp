#pragma once

#include "system/screen_capture_base.hpp"
#include "util/exception.hpp"
#include "util/log.hpp"
#ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <algorithm>
#include <ranges>

namespace bibstd::system
{

///
/// Screen capture class for windows implementation.
///
class screen_capture final : public screen_capture_base
{
public: // Static accessors
  ///
  /// Get the virtual screen metrics.
  /// \return screen metrics
  ///
  static inline auto metrics() -> screen_metrics;

  ///
  /// Capture screen in region defined by x, y, width and height.
  /// \param x Position X of screen area that shall be captured
  /// \param y Position Y of screen area that shall be captured
  /// \param width Width of screen area that shall be captured
  /// \param height Height of screen area that shall be captured
  /// \return captured pixels in screen capture image object
  ///
  static inline auto capture(int x, int y, std::uint32_t width, std::uint32_t height) -> image;
};

///
///
inline auto screen_capture::metrics() -> screen_metrics
{
  return screen_metrics{
    .x = GetSystemMetrics(SM_XVIRTUALSCREEN),
    .y = GetSystemMetrics(SM_YVIRTUALSCREEN),
    .width = static_cast<std::uint32_t>(GetSystemMetrics(SM_CXVIRTUALSCREEN)),
    .height = static_cast<std::uint32_t>(GetSystemMetrics(SM_CYVIRTUALSCREEN))};
}

///
///
inline auto screen_capture::capture(const int x, const int y, const std::uint32_t width, const std::uint32_t height) -> image
{
  static constexpr std::uint16_t bits_per_pixel = 32;
  HWND hwnd = GetDesktopWindow();
  HDC dc = GetDC(hwnd);

  BITMAP bmp = {0};
  HBITMAP h_bmp = reinterpret_cast<HBITMAP>(GetCurrentObject(dc, OBJ_BITMAP));

  if(GetObject(h_bmp, sizeof(BITMAP), &bmp) == 0)
  {
    THROW_EXCEPTION(util::exception("Bitmap dc not found"));
  }

  RECT area = {x, y, x + static_cast<long>(width), y + static_cast<long>(height)};
  HWND window = WindowFromDC(dc);
  GetClientRect(window, &area);

  HDC mem_dc = GetDC(nullptr);
  HDC sdc = CreateCompatibleDC(mem_dc);
  HBITMAP hs_bmp = CreateCompatibleBitmap(mem_dc, width, height);
  DeleteObject(SelectObject(sdc, hs_bmp));
  BitBlt(sdc, 0, 0, width, height, dc, x, y, SRCCOPY);

  const std::uint32_t line_size = width * 4;
  const std::uint32_t data_size = line_size * height;
  std::vector<std::uint8_t> data(data_size);
  BITMAPINFO info = {sizeof(BITMAPINFOHEADER), static_cast<long>(width), static_cast<long>(height), 1, bits_per_pixel, BI_RGB, data_size, 0, 0, 0, 0};
  GetDIBits(sdc, hs_bmp, 0, height, data.data(), &info, DIB_RGB_COLORS);

  std::vector<std::uint8_t> pixels(data_size);
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, height),
    [&](const auto i)
    {
      const auto src_begin = std::next(data.cbegin(), (height - i - 1) * line_size);
      std::copy_n(src_begin, line_size, std::next(pixels.begin(), i * line_size));
    });
  const auto img = image(std::move(pixels), width, height, bits_per_pixel);

  DeleteDC(sdc);
  DeleteObject(hs_bmp);
  ReleaseDC(nullptr, mem_dc);

  ReleaseDC(hwnd, dc);
  return img;
}

} // namespace bibstd::system
