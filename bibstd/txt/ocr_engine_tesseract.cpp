#include "bibstd/txt/ocr_engine_tesseract.hpp"
#include "bibstd/math/coordinates.hpp"
#include "bibstd/system/filesystem.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/visit_helper.hpp"

#include <boost/filesystem/path.hpp>
#include <leptonica/allheaders.h>
#include <leptonica/environ.h>
#include <leptonica/imageio.h>
#include <leptonica/pix_internal.h>
#include <optional>
#include <string>
#include <tesseract/baseapi.h>
#include <tesseract/publictypes.h>

namespace bibstd::txt
{
namespace detail
{

///
/// Forward the pixels struct as a leptonica PIX struct.
/// The pixels object is not copied and must live longer than the PIX object.
/// \param pix Pixels, that shall be referenced to a leptonica PIX struct
///
auto forward_as_pix(auto& data, const std::uint32_t width, const std::uint32_t height) -> Pix
{
  return Pix{
    /*l_uint32          */ width,                                   // width in pixels
    /*l_uint32          */ height,                                  // height in pixels
    /*l_uint32          */ data::pixel::bits_per_pixel,             // depth in bits
    /*l_uint32          */ 4u,                                      // number of samples per pixel
    /*l_uint32          */ width,                                   // 32-bit words/line
    /*l_uint32          */ 1u,                                      // reference count (1 if no clones)
    /*l_int32           */ 0,                                       // image res (ppi) in x direction (use 0 if unknown)
    /*l_int32           */ 0,                                       // image res (ppi) in y direction (use 0 if unknown)
    /*l_int32           */ IFF_UNKNOWN,                             // input file format, IFF_*
    /*l_int32           */ 0,                                       // special instructions for I/O, etc
    /*char              */ nullptr,                                 // text string associated with pix
    /*struct PixColormap*/ nullptr,                                 // colormap (may be null)
    /*l_uint32          */ reinterpret_cast<l_uint32*>(data.data()) // the image data
  };
}

} // namespace detail

///
/// Get the tesseract page iterator level from the resolution tag.
/// \param resolution_tag Tag specifying the OCR resolution
/// \return tesseract page iterator level ID
///
constexpr auto page_iterator_level(ocr_engine_tesseract::resolution_tags resolution_tag) -> tesseract::PageIteratorLevel
{
  using word_tag = ocr_engine_resolution_tag<ocr_engine_tesseract::word>;
  using line_tag = ocr_engine_resolution_tag<ocr_engine_tesseract::line>;
  using paragraph_tag = ocr_engine_resolution_tag<ocr_engine_tesseract::paragraph>;
  return util::visit_lambdas(
    resolution_tag,
    []([[maybe_unused]] word_tag) { return tesseract::RIL_WORD; },
    []([[maybe_unused]] line_tag) { return tesseract::RIL_TEXTLINE; },
    []([[maybe_unused]] paragraph_tag) { return tesseract::RIL_PARA; }
  );
}

///
/// Get the bounding box of the current iterator page object corresponding to the level (word, line, paragraph).
/// \param ri Tesseract page iterator
/// \param level Text resolution level
/// \return bounding box of text object if available, std::nullopt otherwise
///
auto get_bounding_box(const auto& ri, const auto level) -> std::optional<ocr_engine_tesseract::bounding_box_type>
{
  int left{};
  int top{};
  int right{};
  int bottom{};
  const auto found = ri->BoundingBox(level, &left, &top, &right, &bottom);
  if(found && left >= 0 && top >= 0 && right >= 1 && bottom >= 1 && right > left && bottom > top)
  {
    return ocr_engine_tesseract::bounding_box_type(math::coordinates{left, top}, math::coordinates{right, bottom});
  }
  else
  {
    return std::nullopt;
  }
}

///
///
auto ocr_engine_tesseract::tessdata_folder_finder() -> std::optional<std::filesystem::path>
{
  const auto executable_folder_parent = system::filesystem::executable_folder().parent_path();
  const auto root = executable_folder_parent.parent_path();
  const auto best_guess = executable_folder_parent / "share" / "tessdata";
  if(std::filesystem::exists(best_guess))
  {
    return best_guess;
  }
  auto result = std::optional<std::filesystem::path>{};
  const auto is_tessdata_folder = [](const auto& e)
  { return e.is_directory() && e.path().filename() == std::string_view{"tessdata"}; };
  const auto search_folder_from = [&](const auto& root)
  {
    constexpr auto max_search_iterations = 1024;
    auto counter = std::size_t{0};
    auto continue_condition = [&]([[maybe_unused]] const auto&) { return !result || counter++ < max_search_iterations; };
    for(const auto& entry :
        std::filesystem::recursive_directory_iterator{root, std::filesystem::directory_options::skip_permission_denied} |
          std::views::filter(is_tessdata_folder) | std::views::take_while(continue_condition))
    {
      result = entry.path();
    }
  };
  search_folder_from(executable_folder_parent);
  if(!result)
  {
    search_folder_from(root);
  }
  return result;
}

///
///
ocr_engine_tesseract::ocr_engine_tesseract(const std::filesystem::path& tessdata_path, const util::language language)
  : tesseract_{new tesseract::TessBaseAPI()}
{
  if(!std::filesystem::exists(tessdata_path))
  {
    LOG_ERROR("tessdata path does not exist: \"{}\"", tessdata_path.generic_string());
    throw util::exception("non existent tessdata path");
  }
  auto tessdata_string = tessdata_path.generic_string();
  tesseract_->Init(tessdata_string.data(), language_map.at(language).data(), tesseract::OEM_LSTM_ONLY);
  tesseract_->SetVariable("lstm_choice_mode", "2"); // set lstm_choice_mode to alternative symbol choices per character
}

///
///
ocr_engine_tesseract::~ocr_engine_tesseract() noexcept
{
  tesseract_->End();
}

///
///
auto ocr_engine_tesseract::name() const -> name_type
{
  return name_type{"Tesseract"};
}

///
///
auto ocr_engine_tesseract::initialize(
  const pixel_plane_view_type image, const std::optional<pixel_plane_view_type::area_type> subarea
) -> void
{
  if(subarea)
  {
    image_data_.resize(image.data_view_size(*subarea));
    std::ranges::copy(image.data_view(*subarea), image_data_.begin());
  }
  else
  {
    image_data_.resize(image.size());
    std::ranges::copy(image, image_data_.begin());
  }
  // copy needed since pix requires to be non const
  auto pix = detail::forward_as_pix(image_data_, image.width(), image.height());
  tesseract_->SetImage(&pix);
  tesseract_->SetPageSegMode(tesseract::PSM_AUTO_OSD);
}

///
///
auto ocr_engine_tesseract::recognize() const -> recognition_data
{
  if(const auto retval = tesseract_->Recognize(nullptr); retval != 0)
  {
    throw util::exception{std::format("tesseract recognition failure: code={}", retval)};
  }
  std::unique_ptr<tesseract::ResultIterator> ri(tesseract_->GetIterator());
  if(!ri)
  {
    throw util::exception{"tesseract result iterator creation failure"};
  }

  auto result = recognition_data{};

  auto current_paragraph = std::optional<paragraph>{};
  auto current_line = std::optional<line>{};
  auto current_word = std::optional<word>{};

  const auto get_txt = [&ri](const auto level) -> std::optional<std::string>
  {
    std::unique_ptr<char[]> txt(ri->GetUTF8Text(level));
    if(txt)
    {
      return std::string{txt.get()};
    }
    return {};
  };

  const auto get_data = [&ri, &get_txt](auto tag)
  {
    using return_type = std::optional<typename decltype(tag)::underlying_type>;
    auto txt = get_txt(page_iterator_level(tag));
    auto bounding_box = get_bounding_box(ri, page_iterator_level(tag));
    return txt && bounding_box ? typename return_type::value_type{std::move(*txt), std::move(*bounding_box)} : return_type{};
  };

  do
  {
    auto word_data = get_data(tag<word>{});
    if(word_data)
    {
      auto line_data = get_data(tag<line>{});
      auto paragraph_data = get_data(tag<paragraph>{});

      result.emplace_back(
        recognition_data::value_type{
          .word_data = std::move(*word_data), .line_data = std::move(line_data), .paragraph_data = std::move(paragraph_data)
        }
      );
    }
  }
  while(ri->Next(page_iterator_level(tag<word>{})));
  return result;
}

///
///
auto ocr_engine_tesseract::layout_analysis() const -> std::vector<line_layout>
{
  static constexpr auto line_level = page_iterator_level(tag<line>{});
  auto result = std::vector<line_layout>{};
  std::unique_ptr<tesseract::PageIterator> pi(tesseract_->AnalyseLayout(false));
  if(pi)
  {
    do
    {
      if(auto line_bounding_box = get_bounding_box(pi, line_level))
      {
        auto paragraph_bounding_box = get_bounding_box(pi, page_iterator_level(tag<paragraph>{}));
        result.emplace_back(line_layout{std::move(*line_bounding_box), std::move(paragraph_bounding_box)});
      }
    }
    while(pi->Next(line_level));
  }
  return result;
}

} // namespace bibstd::txt
