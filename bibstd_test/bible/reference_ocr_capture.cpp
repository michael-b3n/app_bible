//
// One off capture that turns the images in `BIBSTD_TEST_OCR_DIR` into the OCR data the
// reference_ocr test replays. It runs the real tesseract engine and is therefore hidden from
// ctest: run it explicitly after adding or changing an image.
//
//   bibstd_test.exe "[.capture]"
//
// \see bibstd_test/res/ocr/README.md
//
#include "ocr_capture_data.hpp"

#include <bibstd/data/pixel.hpp>
#include <bibstd/data/plane.hpp>
#include <bibstd/math/rect.hpp>
#include <bibstd/math/value_range.hpp>
#include <bibstd/txt/ocr_engine_tesseract.hpp>
#include <bibstd/util/language.hpp>
#include <bibstd/util/screen_types.hpp>

#include <catch2/catch_test_macros.hpp>

#include <leptonica/allheaders.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>

namespace bibstd::bible
{
namespace
{

///
/// Image formats leptonica reads for us, so that a screenshot can be dropped in as it is.
///
constexpr auto image_extensions = std::array{".png", ".bmp", ".jpg", ".jpeg", ".tif", ".tiff"};

///
/// Read an image of any supported format into a pixel plane.
/// \return Pixel plane with the image content, an empty plane if the image cannot be read
///
auto read_image(const std::filesystem::path& path) -> util::pixel_plane_type
{
  auto* raw = pixRead(path.string().c_str());
  if(raw == nullptr)
  {
    return util::pixel_plane_type{};
  }
  auto* rgb = pixConvertTo32(raw);
  pixDestroy(&raw);
  if(rgb == nullptr)
  {
    return util::pixel_plane_type{};
  }

  const auto width = static_cast<std::uint32_t>(pixGetWidth(rgb));
  const auto height = static_cast<std::uint32_t>(pixGetHeight(rgb));
  const auto words_per_line = static_cast<std::size_t>(pixGetWpl(rgb));
  const auto* const data = pixGetData(rgb);

  auto result = util::pixel_plane_type{width, height};
  for(auto y = std::uint32_t{0}; y < height; ++y)
  {
    const auto* const line = data + (static_cast<std::size_t>(y) * words_per_line);
    for(auto x = std::uint32_t{0}; x < width; ++x)
    {
      result.at((static_cast<std::size_t>(y) * width) + x) = data::pixel{
        .red = static_cast<std::uint8_t>(GET_DATA_BYTE(line, (4 * x) + COLOR_RED)),
        .green = static_cast<std::uint8_t>(GET_DATA_BYTE(line, (4 * x) + COLOR_GREEN)),
        .blue = static_cast<std::uint8_t>(GET_DATA_BYTE(line, (4 * x) + COLOR_BLUE)),
        .alpha = 255
      };
    }
  }
  pixDestroy(&rgb);
  return result;
}

///
/// \return Bounding box of an OCR element as capture box
///
auto to_capture_box(const auto& box) -> test::capture_box
{
  return test::capture_box{
    .x = static_cast<std::int32_t>(box.origin().x()),
    .y = static_cast<std::int32_t>(box.origin().y()),
    .width = static_cast<std::int32_t>(math::size(box.horizontal_range())),
    .height = static_cast<std::int32_t>(math::size(box.vertical_range()))
  };
}

///
/// \return Text and bounding box of an OCR element as capture text
///
auto to_capture_text(const auto& element) -> test::capture_text
{
  return test::capture_text{.text = element.text, .box = to_capture_box(element.bounding_box)};
}

} // namespace

TEST_CASE("capture ocr data of the images in the ocr folder", "[.capture]")
{
  const auto folder = std::filesystem::path{BIBSTD_TEST_OCR_DIR};
  REQUIRE(std::filesystem::is_directory(folder));

  auto images = std::vector<std::filesystem::path>{};
  for(const auto& entry : std::filesystem::directory_iterator{folder})
  {
    const auto extension = entry.path().extension().string();
    if(std::ranges::contains(image_extensions, extension))
    {
      images.push_back(entry.path());
    }
  }
  std::ranges::sort(images);
  REQUIRE_FALSE(images.empty());

  auto engine = txt::ocr_engine_tesseract{std::filesystem::path{BIBSTD_TEST_TESSDATA_DIR}, util::language::german};
  for(const auto& path : images)
  {
    const auto image = read_image(path);
    if(image.empty())
    {
      std::cout << "skipped, image cannot be read: " << path.filename().string() << "\n";
      continue;
    }
    engine.initialize(util::pixel_plane_view_type{image}, std::nullopt);

    auto data = test::capture_data{.id = path.stem().string(), .width = image.width(), .height = image.height()};
    for(const auto& layout : engine.layout_analysis())
    {
      data.layouts.emplace_back(
        test::capture_layout{
          .line = to_capture_box(layout.line_bounding_box),
          .paragraph =
            layout.paragraph_bounding_box ? std::optional{to_capture_box(*layout.paragraph_bounding_box)} : std::nullopt
        }
      );
    }
    for(const auto& element : engine.recognize())
    {
      data.words.emplace_back(
        test::capture_word{
          .word = to_capture_text(element.word_data),
          .line = element.line_data ? std::optional{to_capture_text(*element.line_data)} : std::nullopt,
          .paragraph = element.paragraph_data ? std::optional{to_capture_text(*element.paragraph_data)} : std::nullopt
        }
      );
    }

    const auto capture_path = std::filesystem::path{path}.replace_extension(".ocr");
    CHECK(test::write_capture(capture_path, data));
    std::cout << capture_path.filename().string() << ": " << data.words.size() << " words, " << data.layouts.size()
              << " layout lines, image " << data.width << "x" << data.height << "\n";
  }
}

} // namespace bibstd::bible
