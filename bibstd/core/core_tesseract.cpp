#include "core/core_tesseract.hpp"
#include "data/pix.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/const_bimap.hpp"
#include "util/log.hpp"

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

namespace bibstd::core
{

///
///
core_tesseract::core_tesseract(core_tesseract_common::language language)
  : tesseract_{new tesseract::TessBaseAPI()}
  , pix_{std::make_unique<data::pix>()}
{
  const auto tessdata_string = tessdata_folder_path.generic_string();
  tesseract_->Init(tessdata_string.data(), language_map.at(language).data(), tesseract::OEM_LSTM_ONLY);
}

///
///
core_tesseract::~core_tesseract() noexcept
{
  tesseract_->End();
}

///
///
auto core_tesseract::set_image(const std::function<void(pixel_plane_type&)>& setter) -> void
{
  const auto setter_wrapper = [&](auto& plane) { setter(plane); };
  pix_->update(setter_wrapper);
  tesseract_->SetImage(pix_->get());
  tesseract_->SetPageSegMode(tesseract::PSM_AUTO_OSD);
}

///
///
auto core_tesseract::recognize(std::optional<bounding_box_type> bounding_box) const -> bool
{
  if(bounding_box && !pix_->pixels().empty())
  {
    auto pix_rect = bounding_box_type({0, 0}, pix_->width(), pix_->height());
    const auto overlap = bounding_box_type::overlap(pix_rect, *bounding_box);
    if(!overlap)
    {
      return true;
    }
    pix_rect = overlap.value();
    // Each SetRectangle clears the recognition results so multiple rectangles can be recognized with the same image.
    tesseract_->SetRectangle(
      boost::numeric_cast<int>(pix_rect.origin().x()),
      boost::numeric_cast<int>(pix_rect.origin().y()),
      boost::numeric_cast<int>(pix_rect.horizontal_range()),
      boost::numeric_cast<int>(pix_rect.vertical_range())
    );
    return tesseract_->Recognize(nullptr) == 0;
  }
  else
  {
    return tesseract_->Recognize(nullptr) == 0;
  }
}

///
///
auto core_tesseract::for_each(
  const text_resolution resolution, const std::function<void(std::string_view, const bounding_box_type&)>& do_with_text
) const -> void
{
  for_each_until(
    resolution,
    [&](const auto text, const auto& bounding_box)
    {
      do_with_text(text, bounding_box);
      return false;
    }
  );
}

///
///
auto core_tesseract::for_each_until(
  const text_resolution resolution, const std::function<bool(std::string_view, const bounding_box_type&)>& do_with_text
) const -> void
{
  static constexpr auto resolution_map = util::const_bimap{
    std::pair{text_resolution::character,   tesseract::RIL_SYMBOL},
    std::pair{     text_resolution::word,     tesseract::RIL_WORD},
    std::pair{     text_resolution::line, tesseract::RIL_TEXTLINE},
    std::pair{text_resolution::paragraph,     tesseract::RIL_PARA},
  };
  if(pix_->pixels().empty())
  {
    return;
  }
  std::unique_ptr<tesseract::ResultIterator> ri(tesseract_->GetIterator());
  tesseract::PageIteratorLevel level = resolution_map.at(resolution);
  if(ri)
  {
    auto found = false;
    do
    {
      std::unique_ptr<char[]> txt(ri->GetUTF8Text(level));
      if(txt)
      {
        int left, top, right, bottom;
        const auto success = ri->BoundingBox(level, &left, &top, &right, &bottom);
        if(success && left >= 0 && top >= 0 && right >= 1 && bottom >= 1)
        {
          const auto box = bounding_box_type(bounding_box_type::coordinates_type(left, top), right - left, bottom - top);
          found = do_with_text(std::string_view(txt.get()), box);
        }
        else
        {
          LOG_WARN(log_channel, "Invalid bounding box values: txt={}", txt.get());
        }
      }
    }
    while(!found && (ri->Next(level)));
  }
}

} // namespace bibstd::core
