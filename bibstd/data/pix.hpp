#pragma once

#include "data/pixel.hpp"
#include "data/plane.hpp"
#include <leptonica/environ.h>
#include <leptonica/pix_internal.h>

#include <functional>

namespace bibstd::data
{

///
/// Helper struct as a wrapper for the leptonica PIX struct.
///
class pix final
{
public: // Structors
  pix(std::uint32_t width, std::uint32_t height);
  pix() = default;

  pix(pix&& other) noexcept;
  pix(const pix& other) = delete;

public: // Operators
  auto operator=(pix&& other) & noexcept -> pix&;
  auto operator=(const pix& other) & -> pix& = delete;

public: // Accessors
  ///
  /// Get the width of the pix data.
  /// \return width of the pix
  ///
  auto width() const -> std::uint32_t;

  ///
  /// Get the height of the pix data.
  /// \return height of the pix
  ///
  auto height() const -> std::uint32_t;

  ///
  /// Get a reference to the pix data.
  /// \return reference to the pix data
  ///
  auto pixels() const -> const plane<pixel>::data_type&;

  ///
  /// Get the pointer to the leptonica Pix struct.
  /// \return pointer to the leptonica Pix struct
  ///
  auto get() -> Pix*;

public: // Modifiers
  ///
  /// Update the pix data by providing a new pixel plane. The pix data `size` must be equal to `width * height`.
  /// \param pixel_plane Pixel plane that is used to update the data of pix
  ///
  auto update(plane<pixel>&& pixel_plane) -> void;

private: // Members
  plane<pixel> data_;
  Pix pix_;
};

} // namespace bibstd::data
