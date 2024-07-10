#include "core/core_tesseract.hpp"
#include "system/filesystem.hpp"

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

namespace bibstd::core
{

///
///
core_tesseract::core_tesseract(const std::string_view language)
  : tesseract_(new tesseract::TessBaseAPI())
{
  const auto tessdata = system::filesystem::executable_folder() / "tessdata";
  const auto tessdata_string = tessdata.generic_string();
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
auto core_tesseract::load(util::bitmap&& bitmap) -> void
{
  bitmap_ = std::move(bitmap);
}

///
///
auto core_tesseract::text() -> std::string
{
  const auto rect = bitmap_.dimension();
  tesseract_->SetImage(
    reinterpret_cast<const unsigned char*>(bitmap_.pixels().data()),
    static_cast<int>(rect.horizontal_range()),
    static_cast<int>(rect.vertical_range()),
    util::bitmap::bytes_per_pixel,
    util::bitmap::bytes_per_pixel * static_cast<int>(rect.horizontal_range()));
  tesseract_->Recognize(nullptr);
  tesseract::ResultIterator* ri = tesseract_->GetIterator();
  tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
  if(ri)
  {
    do
    {
      std::unique_ptr<char[]> word(ri->GetUTF8Text(level));
      if(word)
      {
        LOG_WARN("main", "Test Text: {}", std::string{word.get()});
        tesseract::ChoiceIterator ci(*ri);
        do
        {
          [[maybe_unused]] const char* choice = ci.GetUTF8Text();
        }
        while(ci.Next());
      }
    }
    while((ri->Next(level)));
  }
  return {};
}

} // namespace bibstd::core
