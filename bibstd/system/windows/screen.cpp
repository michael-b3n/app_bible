#include "bibstd/system/screen.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/numeric_cast.hpp"
#include "bibstd/util/ranges.hpp"

#include "bibstd/system/windows/win.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <mutex>
#include <ranges>

namespace bibstd::system
{

///
///
auto screen::init() -> bool
{
  // Declare per-monitor DPI awareness for proper screen capture at native resolution.
  // This is also declared in the app manifest. The call here is a no-op if already set.
  SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  return true;
}

///
///
auto screen::metrics() -> screen_rect_type
{
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto x = GetSystemMetrics(SM_XVIRTUALSCREEN);
  const auto y = GetSystemMetrics(SM_YVIRTUALSCREEN);
  const auto width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  const auto height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  return screen_rect_type(math::coordinates{x, y}, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
}

///
///
auto screen::cursor_position() -> screen_coordinates_type
{
  POINT point;
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  if(!GetCursorPos(&point))
  {
    throw util::exception("failed to get cursor position");
  }
  return screen_coordinates_type(point.x, point.y);
}

///
///
auto screen::window_at(const screen_coordinates_type coordinates) -> std::optional<screen_rect_type>
{
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto hwnd = WindowFromPoint(POINT{coordinates.x(), coordinates.y()});
  if(hwnd != nullptr)
  {
    RECT rect;
    if(GetWindowRect(hwnd, &rect))
    {
      return screen_rect_type(
        math::coordinates{numeric_cast<std::int32_t>(rect.left), numeric_cast<std::int32_t>(rect.bottom)},
        math::coordinates{numeric_cast<std::int32_t>(rect.right), numeric_cast<std::int32_t>(rect.top)}
      );
    }
  }
  return std::nullopt;
}

///
///
auto screen::monitor_at(const screen_coordinates_type coordinates) -> std::optional<monitor_type>
{
  SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
  const auto monitor = MonitorFromPoint(POINT{coordinates.x(), coordinates.y()}, MONITOR_DEFAULTTONULL);
  if(monitor == nullptr)
  {
    return std::nullopt;
  }
  // The ansi variant is used since monitor device names are ascii only.
  MONITORINFOEXA info;
  info.cbSize = sizeof(info);
  if(!GetMonitorInfoA(monitor, &info))
  {
    return std::nullopt;
  }
  decltype(auto) rect = info.rcMonitor;
  return monitor_type{
    .rect = screen_rect_type(
      math::coordinates{numeric_cast<std::int32_t>(rect.left), numeric_cast<std::int32_t>(rect.top)},
      math::coordinates{numeric_cast<std::int32_t>(rect.right), numeric_cast<std::int32_t>(rect.bottom)}
    ),
    .device_name = std::string{static_cast<const char*>(info.szDevice)}
  };
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
    const auto hr = numeric_cast<int>(math::size(rect.horizontal_range()));
    const auto vr = numeric_cast<int>(math::size(rect.vertical_range()));

    HDC sdc = CreateCompatibleDC(hdc);
    auto hbitmap = CreateCompatibleBitmap(hdc, hr, vr);
    HGDIOBJ hOld = SelectObject(sdc, hbitmap);
    const auto origin = rect.origin();
    BitBlt(sdc, 0, 0, hr, vr, hdc, origin.x(), origin.y(), SRCCOPY | CAPTUREBLT);
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
  const auto height = numeric_cast<std::uint32_t>(info.bmiHeader.biHeight);
  const auto width = numeric_cast<std::uint32_t>(info.bmiHeader.biWidth);

  pix = pixel_plane_type(width, height);
  std::ranges::for_each(
    util::ranges::index_view_to(height) | std::views::reverse,
    [&, counter = 0u](const auto row_idx) mutable
    {
      std::ranges::for_each(
        util::ranges::index_view_between(width * row_idx, width * (row_idx + 1)),
        [&](const auto index)
        {
          const auto i = counter++ * byte_count;
          auto& p = pix.at(index);
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
