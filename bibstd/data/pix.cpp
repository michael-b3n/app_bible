#include "data/pix.hpp"
#include "util/exception.hpp"
#include <leptonica/imageio.h>

#include <algorithm>
#include <ranges>
#include <string_view>

namespace bibstd::data
{
namespace detail
{

///
/// Forward the pixels struct as a leptonica PIX struct.
/// The pixels object is not copied and must live longer than the PIX object.
/// \param pix Pixels, that shall be referenced to a leptonica PIX struct
///
auto forward_as_pix(plane<pixel>& data) -> Pix
{
  return Pix{
    /*l_uint32          */ data.width,                                   // width in pixels
    /*l_uint32          */ data.height,                                  // height in pixels
    /*l_uint32          */ pixel::bits_per_pixel,                        // depth in bits
    /*l_uint32          */ 4u,                                           // number of samples per pixel
    /*l_uint32          */ data.width,                                   // 32-bit words/line
    /*l_uint32          */ 1u,                                           // reference count (1 if no clones)
    /*l_int32           */ 0,                                            // image res (ppi) in x direction (use 0 if unknown)
    /*l_int32           */ 0,                                            // image res (ppi) in y direction (use 0 if unknown)
    /*l_int32           */ IFF_UNKNOWN,                                  // input file format, IFF_*
    /*l_int32           */ 0,                                            // special instructions for I/O, etc
    /*char              */ nullptr,                                      // text string associated with pix
    /*struct PixColormap*/ nullptr,                                      // colormap (may be null)
    /*l_uint32          */ reinterpret_cast<l_uint32*>(data.data.data()) // the image data
  };
}

} // namespace detail

///
///
pix::pix(std::uint32_t width, std::uint32_t height)
  : data_{width, height}
  , pix_{detail::forward_as_pix(data_)}
{
}

///
///
pix::pix(pix&& other) noexcept
  : data_{std::move(other.data_)}
  , pix_{std::move(other.pix_)}
{
}

///
///
auto pix::operator=(pix&& other) & noexcept -> pix&
{
  data_ = std::move(other.data_);
  pix_ = std::move(other.pix_);
  return *this;
}

///
///
auto pix::width() const -> std::uint32_t
{
  return data_.width;
}

///
///
auto pix::height() const -> std::uint32_t
{
  return data_.height;
}

///
///
auto pix::pixels() const -> const plane<pixel>::data_type&
{
  return data_.data;
}

///
///
auto pix::get() -> Pix*
{
  return &pix_;
}

///
///
auto pix::update(const std::function<void(plane<pixel>&)>& setter) -> void
{
  setter(data_);
  if(data_.data.size() < static_cast<std::size_t>(data_.width) * static_cast<std::size_t>(data_.height))
  {
    THROW_EXCEPTION(std::runtime_error("invalid data update"));
  }
  pix_ = detail::forward_as_pix(data_);
}

} // namespace bibstd::data
