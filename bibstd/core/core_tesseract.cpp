#include "core/core_tesseract.hpp"
#include "util/log.hpp"

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

namespace bibstd::core
{

///
///
core_tesseract::core_tesseract(const std::string_view language)
  : tesseract_(new tesseract::TessBaseAPI())
{
  const auto tessdata_string = tessdata_folder_path.generic_string();
  tesseract_->Init(tessdata_string.data(), language.data(), tesseract::OEM_LSTM_ONLY);
}

///
///
core_tesseract::~core_tesseract() noexcept
{
  tesseract_->End();
}

///
///
auto core_tesseract::set_image(util::bitmap&& bitmap) -> void
{
  bitmap_ = std::move(bitmap);
}

///
///
auto core_tesseract::for_each_word(std::function<void(std::string_view, const bounding_box_type&)> callback) -> void
{
  const auto rect = bitmap_.dimension();
  tesseract_->SetImage(
    reinterpret_cast<const unsigned char*>(bitmap_.pixels().data()),
    static_cast<int>(rect.horizontal_range()),
    static_cast<int>(rect.vertical_range()),
    util::bitmap::bytes_per_pixel,
    util::bitmap::bytes_per_pixel * static_cast<int>(rect.horizontal_range()));
  tesseract_->Recognize(nullptr);
  std::unique_ptr<tesseract::ResultIterator> ri(tesseract_->GetIterator());
  tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
  if(ri)
  {
    do
    {
      std::unique_ptr<char[]> word(ri->GetUTF8Text(level));
      if(word)
      {
        using rect_type = math::rect<std::uint32_t>;
        int left, top, right, bottom;
        ri->BoundingBox(level, &left, &top, &right, &bottom);
        if(left >= 0 && top >= 0 && right >= 1 && bottom >= 1)
        {
          const auto bounding_box = rect_type(rect_type::coordinates_type(left, top), rect_type::coordinates_type(right - 1, bottom - 1));
          callback(std::string_view(word.get()), bounding_box);
        }
        else
        {
          LOG_WARN(log_channel, "Invalid bounding box values: word={}", word.get());
        }
      }
    }
    while((ri->Next(level)));
  }
}

} // namespace bibstd::core
