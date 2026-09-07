#include "bibstd/system/windows/screen_capture.hpp"

namespace bibstd::system
{

///
///
screen_capture::~screen_capture() noexcept = default;

///
///
auto screen_capture::capture(const screen_rect_type rect, pixel_plane_type& pix) -> bool
{
  return do_capture(rect, pix);
}

} // namespace bibstd::system
