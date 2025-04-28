#include "core/core_tesseract.hpp"
#include "data/pix.hpp"
#include "util/boost_numeric_cast.hpp"
#include "util/const_bimap.hpp"
#include "util/log.hpp"

#include <leptonica/allheaders.h>
#include <tesseract/baseapi.h>

namespace bibstd::core
{

// Constants
constexpr auto resolution_map = util::const_bimap{
  std::pair{core_tesseract::text_resolution::character,   tesseract::RIL_SYMBOL},
  std::pair{     core_tesseract::text_resolution::word,     tesseract::RIL_WORD},
  std::pair{     core_tesseract::text_resolution::line, tesseract::RIL_TEXTLINE},
  std::pair{core_tesseract::text_resolution::paragraph,     tesseract::RIL_PARA},
};

///
///
core_tesseract::core_tesseract(core_tesseract_common::language language)
  : tesseract_{new tesseract::TessBaseAPI()}
  , pix_{std::make_unique<data::pix>()}
{
  const auto tessdata_string = tessdata_folder_path.generic_string();
  tesseract_->Init(tessdata_string.data(), language_map.at(language).data(), tesseract::OEM_LSTM_ONLY);
  tesseract_->SetVariable("lstm_choice_mode", "2"); // set lstm_choice_mode to alternative symbol choices per character
}

///
///
core_tesseract::~core_tesseract() noexcept
{
  tesseract_->End();
}

///
///
auto core_tesseract::set_image(pixel_plane_type&& pixel_plane) -> void
{
  pix_->update(std::forward<decltype(pixel_plane)>(pixel_plane));
  tesseract_->SetImage(pix_->get());
  tesseract_->SetPageSegMode(tesseract::PSM_AUTO_OSD);
}

///
///
auto core_tesseract::recognize(std::optional<screen_rect_type> bounding_box) const -> bool
{
  if(bounding_box && !pix_->pixels().empty())
  {
    auto pix_rect = screen_rect_type({0, 0}, pix_->width(), pix_->height());
    const auto overlap = screen_rect_type::overlap(pix_rect, *bounding_box);
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
auto core_tesseract::for_each(const text_resolution resolution, const text_callback_type& do_with_text) const -> void
{
  for_each_while(
    resolution,
    [&](const auto text, const auto& bounding_box)
    {
      do_with_text(text, bounding_box);
      return true;
    }
  );
}

///
///
auto core_tesseract::for_each_while(const text_resolution resolution, const text_while_callback_type& do_with_text) const
  -> void
{
  if(pix_->pixels().empty())
  {
    return;
  }
  std::unique_ptr<tesseract::ResultIterator> ri(tesseract_->GetIterator());
  tesseract::PageIteratorLevel level = resolution_map.at(resolution);
  if(ri)
  {
    auto found = true;
    do
    {
      std::unique_ptr<char[]> txt(ri->GetUTF8Text(level));
      if(txt)
      {
        int left, top, right, bottom;
        const auto success = ri->BoundingBox(level, &left, &top, &right, &bottom);
        if(success && left >= 0 && top >= 0 && right >= 1 && bottom >= 1)
        {
          const auto box = screen_rect_type(screen_coordinates_type(left, top), right - left, bottom - top);
          found = do_with_text(std::string_view(txt.get()), box);
        }
        else
        {
          LOG_WARN("invalid bounding box in for_each_while: txt={}", txt.get());
        }
      }
    }
    while(found && (ri->Next(level)));
  }
}

///
///
auto core_tesseract::for_each_choices(const choices_callback_type& do_with_choices) const -> void
{
  for_each_choices_while(
    [&](const auto& choices, const auto& bounding_box)
    {
      do_with_choices(choices, bounding_box);
      return true;
    }
  );
}

///
///
auto core_tesseract::for_each_choices_while(const choices_while_callback_type& do_with_choices) const -> void
{
  if(pix_->pixels().empty())
  {
    return;
  }
  std::unique_ptr<tesseract::ResultIterator> ri(tesseract_->GetIterator());
  if(ri)
  {
    constexpr auto level = resolution_map.at(text_resolution::character);
    auto found = true;
    auto choices = tesseract_choices{};
    do
    {
      // Get confidence level for alternative symbol choices. Code is based on
      // https://github.com/tesseract-ocr/tesseract/blob/main/src/api/hocrrenderer.cpp#L325-L344
      const auto choice_map = ri->GetBestLSTMSymbolChoices();

      std::unique_ptr<char[]> main_symbol_data(ri->GetUTF8Text(level));
      const auto main_symbol = std::string(main_symbol_data.get());

      choices.clear();
      if(choice_map && !choice_map->empty())
      {
        std::ranges::for_each(
          *choice_map,
          [&](const auto& timesteps)
          {
            std::ranges::for_each(
              timesteps, [&](const auto& choice) { choices.emplace_back(std::string(choice.first), choice.second); }
            );
          }
        );
      }
      choices.emplace_back(main_symbol, ri->Confidence(level));
      std::ranges::sort(choices, [](const auto& a, const auto& b) { return a.confidence > b.confidence; });

      int left, top, right, bottom;
      const auto success = ri->BoundingBox(level, &left, &top, &right, &bottom);
      if(success && left >= 0 && top >= 0 && right >= 1 && bottom >= 1)
      {
        const auto box = screen_rect_type(screen_coordinates_type(left, top), right - left, bottom - top);
        found = do_with_choices(choices, box);
      }
      else
      {
        LOG_WARN("invalid bounding box in for_each_choices_while: main_symbol={}", main_symbol);
      }
    }
    while(found && (ri->Next(level)));
  }
}

} // namespace bibstd::core
