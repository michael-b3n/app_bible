#include "bibstd/system/screen.hpp"
#include "bibstd/system/windows/win.hpp"
#include "bibstd/util/boost_numeric_cast.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <mutex>
#include <ranges>

namespace bibstd::system
{

///
///
auto screen::metrics() -> screen_rect_type
{
  // This should be set with a application manifest. This did not work
  // We set the Dpi awareness explicitly for this process.
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto x = GetSystemMetrics(SM_XVIRTUALSCREEN);
  const auto y = GetSystemMetrics(SM_YVIRTUALSCREEN);
  const auto width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  const auto height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  return screen_rect_type(screen_rect_type::coordinates_type(x, y), width, height);
}

///
///
auto screen::cursor_position() -> screen_coordinates_type
{
  POINT point;
  // This should be set with a application manifest. This did not work
  // We set the Dpi awareness explicitly for this process.
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  if(!GetCursorPos(&point))
  {
    THROW_EXCEPTION(util::exception("failed to get cursor position"));
  }
  return screen_coordinates_type(point.x, point.y);
}

///
///
auto screen::window_at(const screen_coordinates_type coordinates) -> std::optional<screen_rect_type>
{
  // This should be set with a application manifest. This did not work
  // We set the Dpi awareness explicitly for this process.
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto hwnd = WindowFromPoint(POINT{coordinates.x(), coordinates.y()});
  if(hwnd != nullptr)
  {
    RECT rect;
    if(GetWindowRect(hwnd, &rect))
    {
      return screen_rect_type({rect.left, rect.bottom}, {rect.right, rect.top});
    }
  }
  return std::nullopt;
}

///
///
auto screen::capture(const screen_rect_type rect, pixel_plane_type& pix) -> bool
{
  static std::mutex mtx;
  static std::vector<std::byte> pixels_bytes;

  const auto lock = std::lock_guard(mtx);

  HDC hdc = GetDC(nullptr);
  HBITMAP bitmap = [&]
  {
    HDC sdc = CreateCompatibleDC(hdc);
    auto hbitmap = CreateCompatibleBitmap(hdc, rect.horizontal_range(), rect.vertical_range());
    HGDIOBJ hOld = SelectObject(sdc, hbitmap);
    const auto origin = rect.origin();
    BitBlt(sdc, 0, 0, rect.horizontal_range(), rect.vertical_range(), hdc, origin.x(), origin.y(), SRCCOPY | CAPTUREBLT);
    SelectObject(sdc, hOld);
    DeleteDC(sdc);
    return hbitmap;
  }();

  BITMAPINFO info = {};
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  if(0 == GetDIBits(hdc, bitmap, 0, 0, nullptr, &info, DIB_RGB_COLORS))
  {
    LOG_ERROR("capture screen failed: {}", "bitmap info not found");
    return false;
  }
  info.bmiHeader.biCompression = BI_RGB;
  if(pixels_bytes.size() < info.bmiHeader.biSizeImage)
  {
    pixels_bytes.resize(info.bmiHeader.biSizeImage);
  }

  if(0 == GetDIBits(hdc, bitmap, 0, info.bmiHeader.biHeight, static_cast<void*>(pixels_bytes.data()), &info, DIB_RGB_COLORS))
  {
    LOG_ERROR("capture screen failed: {}", "bitmap data not found");
    return false;
  }
  DeleteObject(bitmap);
  ReleaseDC(nullptr, hdc);
  assert(info.bmiHeader.biBitCount >= 24);

  // The loaded pixel bytes are saved to a list of pixels in a row reversed order.
  // This order makes the bitmap data directly compatible with tesseract.
  // Since the Windows screen coordinate system origin is on top left,
  // the highest row is the lowest row in the coordinate system of tesseract,
  // where the origin is on the bottom left.
  const auto byte_count = info.bmiHeader.biBitCount / 8;
  const auto height = boost::numeric_cast<std::uint32_t>(info.bmiHeader.biHeight);
  const auto width = boost::numeric_cast<std::uint32_t>(info.bmiHeader.biWidth);
  pix.width = width;
  pix.height = height;
  if(pix.data.size() < height * width)
  {
    pix.data.resize(height * width);
  }
  std::ranges::for_each(
    std::views::iota(std::size_t{0}, height) | std::views::reverse,
    [&, counter = 0u](const auto row_idx) mutable
    {
      std::ranges::for_each(
        std::views::iota(width * row_idx, width * (row_idx + 1)),
        [&](const auto index)
        {
          const auto i = counter++ * byte_count;
          auto& p = pix.data.at(index);
          p.blue = static_cast<std::uint8_t>(pixels_bytes[i + 0]);
          p.green = static_cast<std::uint8_t>(pixels_bytes[i + 1]);
          p.red = static_cast<std::uint8_t>(pixels_bytes[i + 2]);
        }
      );
    }
  );
  return true;
}

} // namespace bibstd::system
