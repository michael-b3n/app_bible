#pragma once

#include "bibstd/util/screen_types.hpp"

#include <optional>
#include <string>

namespace bibstd::system
{

///
/// Screen capture class for windows implementation.
///
class screen final
{
public: // Typedefs
  using screen_rect_type = util::screen_rect_type;
  using screen_coordinates_type = util::screen_coordinates_type;
  using pixel_plane_type = util::pixel_plane_type;

  ///
  /// Data of a monitor of the virtual screen. The rectangle is given in the screen
  /// coordinate system. The device name identifies the monitor on the platform,
  /// it allows mapping the monitor to the screens known to other libraries.
  ///
  struct monitor_type final
  {
    screen_rect_type rect;
    std::string device_name;
  };

public: // Static accessors
  ///
  /// Set screen related settings. This function must be called once before any other screen function is used.
  /// \return true if initialization was successful, false otherwise
  ///
  static auto init() -> bool;

  ///
  /// Get the virtual screen metrics.
  /// The metrics are given in the screen coordinate system, where the origin is on the top left corner.
  /// \return screen metrics
  ///
  [[nodiscard]] static auto metrics() -> screen_rect_type;

  ///
  /// Get the cursor position in virtual screen coordinate system.
  /// The cursor is givin in the screen coordinate system, where the origin is on the top left corner.
  /// \return screen metrics
  ///
  [[nodiscard]] static auto cursor_position() -> std::optional<screen_coordinates_type>;

  ///
  /// Get the window size at a given position. If no window is found, std::nullopt is returned.
  /// \return screen rectangle
  ///
  [[nodiscard]] static auto window_at(screen_coordinates_type coordinates) -> std::optional<screen_rect_type>;

  ///
  /// Get the monitor at a given position. If no monitor is found, std::nullopt is returned.
  /// \return monitor data
  ///
  [[nodiscard]] static auto monitor_at(screen_coordinates_type coordinates) -> std::optional<monitor_type>;

  ///
  /// Capture screen in region defined by a rectangle. The rectangle shall be in the
  /// screen coordinate system, where the origin is on the top left corner.
  /// A rectangle of screen area that shall be captured and the pixels object to save the
  /// captured pixels must be provided. The pixels are saved row by row, the topmost row of
  /// the region first (which is the order e.g. tesseract reads them in).
  ///
  [[nodiscard]] static auto capture(screen_rect_type rect, pixel_plane_type& pix) -> bool;
};

} // namespace bibstd::system
