#include "test_utils/ocr_capture_data.hpp"

#include <bibstd/bible/reference_ocr.hpp>
#include <bibstd/data/pixel.hpp>
#include <bibstd/data/plane.hpp>
#include <bibstd/math/coordinates.hpp>
#include <bibstd/math/rect.hpp>
#include <bibstd/math/value_range.hpp>
#include <bibstd/txt/ocr_engine.hpp>
#include <bibstd/util/scope_guard.hpp>
#include <bibstd/util/screen_types.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <format>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace bibstd::bible
{
namespace
{

using area_type = util::pixel_plane_view_type::area_type;

///
/// \return Capture box as screen rect
///
auto to_rect(const test_utils::capture_box& box) -> util::screen_rect_type
{
  return util::screen_rect_type{
    math::coordinates(box.x, box.y), static_cast<std::uint32_t>(box.width), static_cast<std::uint32_t>(box.height)
  };
}

///
/// \return Screen rect moved by the given offset
///
auto shifted(const util::screen_rect_type& box, const util::screen_coordinates_type offset) -> util::screen_rect_type
{
  return util::screen_rect_type{box.origin() + offset, math::size(box.horizontal_range()), math::size(box.vertical_range())};
}

///
/// \return Centre of a capture box, the position a user would point at to hit the word
///
auto centre(const test_utils::capture_box& box) -> reference_ocr::position_type
{
  return {box.x + (box.width / 2), box.y + (box.height / 2)};
}

///
/// OCR engine that replays captured data instead of looking at the image. Recognition honours the
/// subarea the same way a real engine does: only overlapping words are reported and their boxes are
/// relative to the subarea, so that reference_ocr has to shift them back itself.
///
class capture_engine final : public txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>
{
  // Variables
  test_utils::capture_data data_;
  std::optional<area_type> subarea_;

public: // Constants
  static constexpr auto default_name = "capture";

public: // Structors
  explicit capture_engine(test_utils::capture_data data)
    : data_{std::move(data)}
  {
  }

public: // Accessors
  ///
  /// \return Subarea the engine was initialized with last
  ///
  auto last_subarea() const -> const std::optional<area_type>& { return subarea_; }

public: // Overrides
  auto name() const -> name_type override { return name_type{default_name}; }

  auto initialize([[maybe_unused]] pixel_plane_view_type image, std::optional<area_type> subarea) -> void override
  {
    subarea_ = subarea;
  }

  auto recognize() const -> recognition_data override
  {
    const auto offset = reported_offset();
    const auto to_word = [&](const test_utils::capture_text& t) { return word{t.text, shifted(to_rect(t.box), offset)}; };
    const auto to_line = [&](const test_utils::capture_text& t) { return line{t.text, shifted(to_rect(t.box), offset)}; };
    const auto to_paragraph = [&](const test_utils::capture_text& t)
    { return paragraph{t.text, shifted(to_rect(t.box), offset)}; };

    auto result = recognition_data{};
    for(const auto& element : data_.words)
    {
      if(!recognized(element.word.box))
      {
        continue;
      }
      result.emplace_back(
        recognition_data_element{
          .word_data = to_word(element.word),
          .line_data = element.line ? std::optional{to_line(*element.line)} : std::nullopt,
          .paragraph_data = element.paragraph ? std::optional{to_paragraph(*element.paragraph)} : std::nullopt
        }
      );
    }
    return result;
  }

  auto layout_analysis() const -> std::vector<line_layout> override
  {
    auto result = std::vector<line_layout>{};
    for(const auto& layout : data_.layouts)
    {
      result.emplace_back(
        line_layout{
          .line_bounding_box = to_rect(layout.line),
          .paragraph_bounding_box = layout.paragraph ? std::optional{to_rect(*layout.paragraph)} : std::nullopt
        }
      );
    }
    return result;
  }

private: // Implementation
  ///
  /// A real engine recognizes the subarea clipped to the image, so a subarea reaching over an image
  /// edge does not shift the result.
  /// \return Subarea the engine works on, std::nullopt when the whole image is recognized
  ///
  auto clipped_subarea() const -> std::optional<area_type>
  {
    const auto image_box = test_utils::capture_box{
      .width = static_cast<std::int32_t>(data_.width), .height = static_cast<std::int32_t>(data_.height)
    };
    return subarea_ ? math::overlap(*subarea_, area_type{to_rect(image_box)}) : std::nullopt;
  }

  ///
  /// \return Offset turning image coordinates into the coordinates the engine reports its boxes in
  ///
  auto reported_offset() const -> util::screen_coordinates_type
  {
    const auto clipped = clipped_subarea();
    return clipped ? util::screen_coordinates_type{
                       -static_cast<util::screen_rect_type::value_type>(clipped->origin().x()),
                       -static_cast<util::screen_rect_type::value_type>(clipped->origin().y())
                     }
                   : util::screen_coordinates_type{0, 0};
  }

  ///
  /// \return true if the element is part of the recognized area
  ///
  auto recognized(const test_utils::capture_box& box) const -> bool
  {
    if(!subarea_)
    {
      return true;
    }
    const auto clipped = clipped_subarea();
    return clipped && math::overlap(*clipped, area_type{to_rect(box)}).has_value();
  }
};

///
/// OCR engine replaying a capture without its paragraphs, the shape a system engine has.
///
class line_capture_engine final : public txt::ocr_engine<txt::ocr_engine_tag_plain>
{
  // Variables
  capture_engine engine_;

public: // Constants
  static constexpr auto default_name = "lines";

public: // Structors
  explicit line_capture_engine(test_utils::capture_data data)
    : engine_{std::move(data)}
  {
  }

public: // Overrides
  auto name() const -> name_type override { return name_type{default_name}; }

  auto initialize(pixel_plane_view_type image, std::optional<area_type> subarea) -> void override
  {
    engine_.initialize(image, subarea);
  }

  auto recognize() const -> recognition_data override
  {
    auto result = recognition_data{};
    for(const auto& element : engine_.recognize())
    {
      result.emplace_back(recognition_data_element{.word_data = element.word_data, .line_data = element.line_data});
    }
    return result;
  }
};

///
/// OCR engine without layout analysis support, to reach the paths that require one.
///
class plain_engine final : public txt::ocr_engine<txt::ocr_engine_tag_plain>
{
public: // Constants
  static constexpr auto default_name = "plain";

public: // Overrides
  auto name() const -> name_type override { return name_type{default_name}; }

  auto initialize([[maybe_unused]] pixel_plane_view_type image, [[maybe_unused]] std::optional<area_type> subarea)
    -> void override
  {
  }

  auto recognize() const -> recognition_data override { return recognition_data{}; }
};

///
/// A designed capture with two paragraphs: the first holds two lines, the second holds one. The
/// numbers are round so that the areas the paragraph recognition derives can be written down by hand.
///
///   y= 50  Der Vers Johannes 3,     |
///   y= 80  16 ist bekannt.          | paragraph 1
///   y=120  Ein anderer Absatz.        paragraph 2
///
struct designed_capture final
{
  // Constants
  static constexpr auto line_height = std::int32_t{20};
  static constexpr auto paragraph_1_text = "Der Vers Johannes 3,\n16 ist bekannt.\n";
  static constexpr auto paragraph_2_text = "Ein anderer Absatz.\n";

  // Variables
  test_utils::capture_data data;

  ///
  /// \see designed_capture
  ///
  designed_capture()
  {
    const auto text_of = [](const std::string& text, const std::int32_t x, const std::int32_t y, const std::int32_t width)
    {
      return test_utils::capture_text{
        .text = text, .box = {.x = x, .y = y, .width = width, .height = line_height}
      };
    };
    const auto line1 = text_of("Der Vers Johannes 3,\n", 50, 50, 200);
    const auto line2 = text_of("16 ist bekannt.\n", 50, 80, 200);
    const auto line3 = text_of("Ein anderer Absatz.\n", 50, 120, 200);
    const auto paragraph1 = test_utils::capture_text{
      .text = line1.text + line2.text, .box = {.x = 50, .y = 50, .width = 200, .height = 50}
    };
    const auto paragraph2 = test_utils::capture_text{.text = line3.text, .box = line3.box};

    data = test_utils::capture_data{
      .id = "designed",
      .width = 300,
      .height = 180,
      .layouts =
        {test_utils::capture_layout{.line = line1.box, .paragraph = paragraph1.box},
                  test_utils::capture_layout{.line = line2.box, .paragraph = paragraph1.box},
                  test_utils::capture_layout{.line = line3.box, .paragraph = paragraph2.box}},
      .words = {
                  test_utils::capture_word{.word = text_of("Der", 50, 50, 30), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("Vers", 90, 50, 40), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("Johannes", 140, 50, 80), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("3,", 230, 50, 20), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("16", 50, 80, 20), .line = line2, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("ist", 80, 80, 30), .line = line2, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("bekannt.", 120, 80, 80), .line = line2, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("Ein", 50, 120, 30), .line = line3, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("anderer", 90, 120, 70), .line = line3, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("Absatz.", 170, 120, 70), .line = line3, .paragraph = paragraph2}
      }
    };
  }

  ///
  /// \return Box of the word with the given text
  ///
  auto word(const std::string_view text) const -> test_utils::capture_box
  {
    const auto it = std::ranges::find(data.words, text, [](const auto& w) { return std::string_view{w.word.text}; });
    REQUIRE(it != std::ranges::cend(data.words));
    return it->word.box;
  }

  ///
  /// Move the whole capture, so that the area the paragraph recognition asks for reaches over the image edge.
  ///
  auto move_by(const std::int32_t offset) -> void
  {
    const auto move = [offset](test_utils::capture_box& box)
    {
      box.x -= offset;
      box.y -= offset;
    };
    for(auto& layout : data.layouts)
    {
      move(layout.line);
      if(layout.paragraph)
      {
        move(*layout.paragraph);
      }
    }
    for(auto& element : data.words)
    {
      move(element.word.box);
      if(element.line)
      {
        move(element.line->box);
      }
      if(element.paragraph)
      {
        move(element.paragraph->box);
      }
    }
  }
};

///
/// A designed capture whose paragraph repeats a word of the line above it and words of its own.
/// The word above carries a descender reaching into the area the paragraph recognition asks for,
/// so it is recognized together with the paragraph although it supplies none of its text.
///
///   y= 20  Was hier gilt.            paragraph 1, "gilt." reaching down to y=55
///   y= 50  Der Vers Johannes 3,    |
///   y= 80  16 ist bekannt.         | paragraph 2
///   y=110  Der Vers gilt.          |
///
struct repeated_word_capture final
{
  // Constants
  static constexpr auto line_height = std::int32_t{20};
  static constexpr auto paragraph_2_text = "Der Vers Johannes 3,\n16 ist bekannt.\nDer Vers gilt.\n";

  // Variables
  test_utils::capture_data data;

  ///
  /// \see repeated_word_capture
  ///
  repeated_word_capture()
  {
    const auto text_of = [](
                           const std::string& text,
                           const std::int32_t x,
                           const std::int32_t y,
                           const std::int32_t width,
                           const std::int32_t height = line_height
                         )
    {
      return test_utils::capture_text{
        .text = text, .box = {.x = x, .y = y, .width = width, .height = height}
      };
    };
    const auto line1 = text_of("Was hier gilt.\n", 50, 20, 200);
    const auto line2 = text_of("Der Vers Johannes 3,\n", 50, 50, 200);
    const auto line3 = text_of("16 ist bekannt.\n", 50, 80, 200);
    const auto line4 = text_of("Der Vers gilt.\n", 50, 110, 200);
    const auto paragraph1 = test_utils::capture_text{.text = line1.text, .box = line1.box};
    const auto paragraph2 = test_utils::capture_text{
      .text = line2.text + line3.text + line4.text, .box = {.x = 50, .y = 50, .width = 200, .height = 80}
    };

    data = test_utils::capture_data{
      .id = "repeated word",
      .width = 300,
      .height = 200,
      .layouts =
        {test_utils::capture_layout{.line = line1.box, .paragraph = paragraph1.box},
                  test_utils::capture_layout{.line = line2.box, .paragraph = paragraph2.box},
                  test_utils::capture_layout{.line = line3.box, .paragraph = paragraph2.box},
                  test_utils::capture_layout{.line = line4.box, .paragraph = paragraph2.box}},
      .words = {
                  test_utils::capture_word{.word = text_of("Was", 50, 20, 30), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("hier", 90, 20, 40), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("gilt.", 140, 20, 50, 35), .line = line1, .paragraph = paragraph1},
                  test_utils::capture_word{.word = text_of("Der", 50, 50, 30), .line = line2, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("Vers", 90, 50, 40), .line = line2, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("Johannes", 140, 50, 80), .line = line2, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("3,", 230, 50, 20), .line = line2, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("16", 50, 80, 20), .line = line3, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("ist", 80, 80, 30), .line = line3, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("bekannt.", 120, 80, 80), .line = line3, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("Der", 50, 110, 30), .line = line4, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("Vers", 90, 110, 50), .line = line4, .paragraph = paragraph2},
                  test_utils::capture_word{.word = text_of("gilt.", 150, 110, 50), .line = line4, .paragraph = paragraph2}
      }
    };
  }

  ///
  /// \return Boxes of all words with the given text, in reading order
  ///
  auto word_boxes(const std::string_view text) const -> std::vector<test_utils::capture_box>
  {
    return data.words | std::views::filter([&](const auto& w) { return w.word.text == text; }) |
           std::views::transform([](const auto& w) { return w.word.box; }) | std::ranges::to<std::vector>();
  }

  ///
  /// \return Box of the word with the given text, the first one if the text is repeated
  ///
  auto word(const std::string_view text) const -> test_utils::capture_box
  {
    const auto it = std::ranges::find(data.words, text, [](const auto& w) { return std::string_view{w.word.text}; });
    REQUIRE(it != std::ranges::cend(data.words));
    return it->word.box;
  }
};

///
/// One capture, the blank image belonging to it and the engines replaying it. The pixels are never
/// looked at, only the dimensions are, because the recognition clips its area to the image.
///
class ocr_driver final
{
  // Variables
  util::pixel_plane_type image_;
  reference_ocr::ocr_engine_list_type engines_;
  // Adding engines may move the list, the engines themselves stay put behind their unique_ptr.
  const capture_engine* capture_engine_{nullptr};

public: // Structors
  explicit ocr_driver(const test_utils::capture_data& capture)
    : image_{capture.width, capture.height}
  {
    auto engine = std::make_unique<capture_engine>(capture);
    capture_engine_ = engine.get();
    engines_.emplace_back(std::move(engine));
  }

public: // Accessors
  ///
  /// \return Engine list, to add further engines or to hand it to reference_ocr directly
  ///
  auto engines() -> reference_ocr::ocr_engine_list_type& { return engines_; }

  ///
  /// \return Blank image with the dimensions of the capture
  ///
  auto image() const -> util::pixel_plane_view_type { return util::pixel_plane_view_type{image_}; }

  ///
  /// \return Subarea the replaying engine was initialized with last
  ///
  auto last_subarea() const -> const std::optional<area_type>& { return capture_engine_->last_subarea(); }

public: // Operations
  ///
  /// \return Algorithm data driving both algorithms with the replaying engine
  ///
  static auto algorithm_data(const reference_ocr::algorithm_type algorithm) -> reference_ocr::algorithm_data
  {
    return reference_ocr::algorithm_data{
      .algorithm = algorithm,
      .engine_name_character_recognition = capture_engine::default_name,
      .engine_name_layout_recognition = capture_engine::default_name
    };
  }

  ///
  /// Run the recognition at the given position.
  /// \return Result of \see reference_ocr::run
  ///
  auto run(
    const reference_ocr::position_type position,
    const reference_ocr::algorithm_type algorithm = reference_ocr::algorithm_type::recognize_with_paragraph_recognition
  )
  {
    return reference_ocr::run(engines_, image(), position, algorithm_data(algorithm));
  }

  ///
  /// Run the recognition at the centre of the given box.
  /// \return Result of \see reference_ocr::run
  ///
  auto run_at(
    const test_utils::capture_box& box,
    const reference_ocr::algorithm_type algorithm = reference_ocr::algorithm_type::recognize_with_paragraph_recognition
  )
  {
    return run(centre(box), algorithm);
  }
};

} // namespace

TEST_CASE("reference_ocr resolves the position to a character of the paragraph", "[bible]")
{
  const auto capture = designed_capture{};
  auto driver = ocr_driver{capture.data};

  SECTION("a position on a word of the first paragraph")
  {
    const auto result = driver.run_at(capture.word("Johannes"));
    REQUIRE(result.has_value());

    // Both lines belong to one paragraph, so the whole paragraph is returned, line break included.
    CHECK(result->text == designed_capture::paragraph_1_text);
    CHECK(result->character_bounding_boxes.size() == result->text.size());

    // The cursor character has to land inside the word that was pointed at.
    const auto word_begin = result->text.find("Johannes");
    REQUIRE(word_begin != std::string::npos);
    CHECK(result->cursor_character_index >= word_begin);
    CHECK(result->cursor_character_index < word_begin + std::string_view{"Johannes"}.size());
  }
  SECTION("a position on a word of the second paragraph")
  {
    const auto result = driver.run_at(capture.word("Absatz."));
    REQUIRE(result.has_value());
    CHECK(result->text == designed_capture::paragraph_2_text);
  }
  SECTION("a position outside every line")
  {
    const auto result = driver.run(math::coordinates(290, 175));
    REQUIRE(result.has_value());
    CHECK(result->text.empty());
    CHECK(result->character_bounding_boxes.empty());
  }
  SECTION("recognition without layout analysis")
  {
    const auto result =
      driver.run_at(capture.word("Johannes"), reference_ocr::algorithm_type::recognize_just_with_line_recognition);
    REQUIRE(result.has_value());

    // Without a subarea the whole image is recognized, but the reported element is the same.
    CHECK(result->text == designed_capture::paragraph_1_text);
    CHECK(result->character_bounding_boxes.size() == result->text.size());
  }
}

TEST_CASE("reference_ocr keeps the lines around the position when the engine reports no paragraph", "[bible]")
{
  // A reference may be broken over a line break, so a line only engine would cut it in half.
  const auto capture = designed_capture{};
  auto driver = ocr_driver{capture.data};
  driver.engines().emplace_back(std::make_unique<line_capture_engine>(capture.data));

  auto ad = ocr_driver::algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
  ad.engine_name_character_recognition = line_capture_engine::default_name;

  SECTION("both lines of the recognized paragraph")
  {
    const auto result = reference_ocr::run(driver.engines(), driver.image(), centre(capture.word("Johannes")), ad);
    REQUIRE(result.has_value());
    CHECK(result->text == designed_capture::paragraph_1_text);
    CHECK(result->character_bounding_boxes.size() == result->text.size());
  }
  SECTION("without layout analysis only the line above and below")
  {
    ad.algorithm = reference_ocr::algorithm_type::recognize_just_with_line_recognition;
    const auto result = reference_ocr::run(driver.engines(), driver.image(), centre(capture.word("anderer")), ad);
    REQUIRE(result.has_value());

    // The whole image is recognized here, so the first line stays out of the text.
    CHECK(result->text == "16 ist bekannt.\nEin anderer Absatz.\n");
  }
}

TEST_CASE("reference_ocr widens the recognition area to the paragraph", "[bible]")
{
  const auto capture = designed_capture{};
  auto driver = ocr_driver{capture.data};

  SECTION("a position on the first line of a two line paragraph")
  {
    std::ignore = driver.run_at(capture.word("Der"));
    REQUIRE(driver.last_subarea().has_value());
    const auto& area = *driver.last_subarea();

    // The next line of the same paragraph is taken in, then half a line height is added as padding.
    CHECK(area.origin().x() == 40);
    CHECK(area.origin().y() == 40);
    CHECK(math::size(area.horizontal_range()) == 220);
    CHECK(math::size(area.vertical_range()) == 70);
  }
  SECTION("a position on a paragraph of a single line")
  {
    std::ignore = driver.run_at(capture.word("Ein"));
    REQUIRE(driver.last_subarea().has_value());
    const auto& area = *driver.last_subarea();

    // There is no neighbouring line of the same paragraph, so only the padding is added.
    CHECK(area.origin().x() == 40);
    CHECK(area.origin().y() == 110);
    CHECK(math::size(area.horizontal_range()) == 220);
    CHECK(math::size(area.vertical_range()) == 40);
  }
}

TEST_CASE("reference_ocr reports character boxes in image coordinates", "[bible]")
{
  // The character recognition runs on a subarea and reports boxes relative to it, so the result is
  // only usable after reference_ocr shifted them back into the coordinate system of the image.
  const auto capture = designed_capture{};
  auto driver = ocr_driver{capture.data};

  const auto johannes = capture.word("Johannes");
  const auto result = driver.run_at(johannes);
  REQUIRE(result.has_value());
  REQUIRE(result->cursor_character_index < result->character_bounding_boxes.size());

  const auto& cursor_box = result->character_bounding_boxes.at(result->cursor_character_index);
  REQUIRE(cursor_box.has_value());
  CHECK(math::overlap(*cursor_box, to_rect(johannes)).has_value());
}

TEST_CASE("reference_ocr reports character boxes of a paragraph at the image edge", "[bible]")
{
  // The padding the paragraph recognition adds reaches over the image edge here, so the recognized
  // area is clipped. The boxes are relative to the clipped area and must still come back in image
  // coordinates.
  auto capture = designed_capture{};
  capture.move_by(46);
  auto driver = ocr_driver{capture.data};

  const auto johannes = capture.word("Johannes");
  const auto result = driver.run_at(johannes);
  REQUIRE(result.has_value());

  // The area asked for starts above the image, so the engine sees it clipped to the image edge.
  REQUIRE(driver.last_subarea().has_value());
  CHECK(driver.last_subarea()->origin().x() == 0);
  CHECK(driver.last_subarea()->origin().y() == 0);

  // The first character of the paragraph is the first character of its first word, so its box
  // pins the coordinate system the boxes are reported in.
  REQUIRE_FALSE(result->character_bounding_boxes.empty());
  const auto& first_box = result->character_bounding_boxes.front();
  REQUIRE(first_box.has_value());
  CHECK(first_box->origin().x() == capture.data.words.front().word.box.x);
  CHECK(first_box->origin().y() == capture.data.words.front().word.box.y);

  REQUIRE(result->cursor_character_index < result->character_bounding_boxes.size());
  const auto& cursor_box = result->character_bounding_boxes.at(result->cursor_character_index);
  REQUIRE(cursor_box.has_value());
  CHECK(math::overlap(*cursor_box, to_rect(johannes)).has_value());
}

TEST_CASE("reference_ocr keeps a word of a neighbouring line out of the text it did not supply", "[bible]")
{
  // The recognized area reaches a bit beyond the paragraph, so words of the lines around it are
  // recognized too. Such a word must not claim the characters of a word of the same spelling
  // inside the text: it would take the character positions of the words following it with it.
  const auto capture = repeated_word_capture{};
  auto driver = ocr_driver{capture.data};
  driver.engines().emplace_back(std::make_unique<line_capture_engine>(capture.data));

  auto ad = ocr_driver::algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
  SECTION("an engine reporting paragraphs")
  {
    ad.engine_name_character_recognition = capture_engine::default_name;
  }
  SECTION("an engine reporting lines only")
  {
    ad.engine_name_character_recognition = line_capture_engine::default_name;
  }

  const auto bekannt = capture.word("bekannt.");
  const auto result = reference_ocr::run(driver.engines(), driver.image(), centre(bekannt), ad);
  REQUIRE(result.has_value());
  CHECK(result->text == repeated_word_capture::paragraph_2_text);
  REQUIRE(result->character_bounding_boxes.size() == result->text.size());

  // The cursor character has to land inside the word that was pointed at, it sits on the line
  // between the two lines the repeated word could have dragged the character positions across.
  const auto word_begin = result->text.find("bekannt.");
  REQUIRE(word_begin != std::string::npos);
  CHECK(result->cursor_character_index >= word_begin);
  CHECK(result->cursor_character_index < word_begin + std::string_view{"bekannt."}.size());

  // The repeated word carries the box of the word inside the paragraph, not the one above it.
  const auto repeated_begin = result->text.rfind("gilt.");
  REQUIRE(repeated_begin != std::string::npos);
  const auto& repeated_box = result->character_bounding_boxes.at(repeated_begin);
  REQUIRE(repeated_box.has_value());
  CHECK(math::overlap(*repeated_box, to_rect(capture.data.words.back().word.box)).has_value());

  // A word repeated within the text takes the occurrence belonging to it, so the boxes of the two
  // occurrences stay on the line each of them was recognized on.
  const auto boxes_of = capture.word_boxes("Vers");
  REQUIRE(boxes_of.size() == 2);
  const auto first_begin = result->text.find("Vers");
  const auto second_begin = result->text.rfind("Vers");
  REQUIRE(first_begin != std::string::npos);
  REQUIRE(first_begin != second_begin);
  const auto& first_box = result->character_bounding_boxes.at(first_begin);
  const auto& second_box = result->character_bounding_boxes.at(second_begin);
  REQUIRE(first_box.has_value());
  REQUIRE(second_box.has_value());
  CHECK(math::overlap(*first_box, to_rect(boxes_of.front())).has_value());
  CHECK(math::overlap(*second_box, to_rect(boxes_of.back())).has_value());
}

TEST_CASE("reference_ocr reports no position data when the pointed at line stays unlocated", "[bible]")
{
  // An engine may report a word that is not part of the text of its own line. Without a single
  // located character of the line that was pointed at there is no cursor index, and the closest
  // one is the first character of the text, a reference the user never pointed at.
  auto capture = designed_capture{};
  const auto absatz = capture.word("Absatz.");
  for(auto& element : capture.data.words)
  {
    if(element.word.box.y == absatz.y)
    {
      element.word.text = "###";
    }
  }
  auto driver = ocr_driver{capture.data};

  const auto result = driver.run_at(absatz);
  REQUIRE(result.has_value());
  CHECK(result->text.empty());
  CHECK(result->character_bounding_boxes.empty());
}

TEST_CASE("reference_ocr rejects unusable engine setups", "[bible]")
{
  const auto capture = designed_capture{};
  auto driver = ocr_driver{capture.data};
  const auto position = centre(capture.word("Johannes"));

  auto ad = ocr_driver::algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
  auto expected_error = reference_ocr::unexpected_ocr_result::error;

  SECTION("no engine with the requested character recognition name")
  {
    ad.engine_name_character_recognition = "not the name of any engine";
  }
  SECTION("no engine for the layout recognition")
  {
    ad.engine_name_layout_recognition = std::nullopt;
  }
  SECTION("an engine without layout analysis support for the paragraph recognition")
  {
    driver.engines().emplace_back(std::make_unique<plain_engine>());
    ad.engine_name_layout_recognition = plain_engine::default_name;
  }
  SECTION("an undefined engine")
  {
    driver.engines().emplace_back(std::monostate{});
    ad.algorithm = reference_ocr::algorithm_type::recognize_just_with_line_recognition;
    ad.engine_name_character_recognition = "Undefined";
  }
  SECTION("an unsupported algorithm")
  {
    ad.algorithm = static_cast<reference_ocr::algorithm_type>(-1);
    expected_error = reference_ocr::unexpected_ocr_result::unsupported;
  }

  const auto result = reference_ocr::run(driver.engines(), driver.image(), position, ad);
  REQUIRE_FALSE(result.has_value());
  CHECK(result.error() == expected_error);
}

TEST_CASE("capture data survives a write and read cycle", "[bible]")
{
  const auto path = std::filesystem::temp_directory_path() / "bibstd_test_capture.ocr";
  const auto guard = util::scope_guard{[&] { std::filesystem::remove(path); }};

  const auto capture = designed_capture{};
  REQUIRE(test_utils::write_capture(path, capture.data));

  SECTION("everything the capture holds comes back")
  {
    const auto read = test_utils::read_capture(path);
    REQUIRE(read.has_value());
    CHECK(read->id == path.stem().string());
    CHECK(read->width == capture.data.width);
    CHECK(read->height == capture.data.height);

    REQUIRE(read->layouts.size() == capture.data.layouts.size());
    for(const auto& [expected, actual] : std::views::zip(capture.data.layouts, read->layouts))
    {
      CHECK(actual.line == expected.line);
      CHECK(actual.paragraph == expected.paragraph);
    }

    REQUIRE(read->words.size() == capture.data.words.size());
    for(const auto& [expected, actual] : std::views::zip(capture.data.words, read->words))
    {
      // The line breaks of the line and paragraph texts have to survive the escaping.
      CHECK(actual.word == expected.word);
      CHECK(actual.line == expected.line);
      CHECK(actual.paragraph == expected.paragraph);
    }
  }
  SECTION("a capture killed mid write is rejected")
  {
    // Cutting the file leaves a record without its numbers, which is what a killed capture leaves behind.
    const auto size = std::filesystem::file_size(path);
    REQUIRE(size > 20);
    std::filesystem::resize_file(path, size - 20);
    CHECK_FALSE(test_utils::read_capture(path).has_value());
  }
  SECTION("a missing file is no capture")
  {
    CHECK_FALSE(test_utils::read_capture(path.parent_path() / "no_such_capture.ocr").has_value());
  }
}

TEST_CASE("reference_ocr handles captured screenshots", "[bible]")
{
  // The captures are produced from local screenshots and are not part of the repository,
  // \see bibstd_test/res/ocr/README.md.
  const auto captures = test_utils::read_captures(std::filesystem::path{BIBSTD_TEST_OCR_DIR});
  if(captures.empty())
  {
    SKIP(std::format("no ocr captures in {}", BIBSTD_TEST_OCR_DIR));
  }

  for(const auto& capture : captures)
  {
    INFO(std::format("capture: {}", capture.id));
    const auto image_area = util::screen_rect_type{math::coordinates(0, 0), capture.width, capture.height};

    // run() re-initializes the engine on every call, so one driver serves the whole capture.
    auto driver = ocr_driver{capture};
    driver.engines().emplace_back(std::make_unique<line_capture_engine>(capture));

    // The characters are recognized by the engine of the system, which reports no paragraphs. Only
    // the layout analysis contributes them, so both shapes have to come back with the same quality.
    for(const auto& character_recognition : {capture_engine::default_name, line_capture_engine::default_name})
    {
      INFO(std::format("character recognition: {}", character_recognition));
      auto ad = ocr_driver::algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
      ad.engine_name_character_recognition = character_recognition;

      for(const auto& element : capture.words)
      {
        INFO(std::format("word: \"{}\"", element.word.text));
        const auto result = reference_ocr::run(driver.engines(), driver.image(), centre(element.word.box), ad);
        REQUIRE(result.has_value());

        // One box per character, so that the caller can address every character of the text.
        REQUIRE(result->character_bounding_boxes.size() == result->text.size());
        if(result->text.empty())
        {
          continue;
        }
        REQUIRE(result->cursor_character_index < result->text.size());

        // Every reported box has been shifted back into the coordinate system of the image.
        const auto boxes_within_image = std::ranges::all_of(
          result->character_bounding_boxes, [&](const auto& box) { return !box || math::overlap(image_area, *box).has_value(); }
        );
        CHECK(boxes_within_image);

        // The character the cursor resolved to belongs to the word that was pointed at.
        const auto& cursor_box = result->character_bounding_boxes.at(result->cursor_character_index);
        REQUIRE(cursor_box.has_value());
        CHECK(math::overlap(*cursor_box, to_rect(element.word.box)).has_value());
      }
    }
  }
}

} // namespace bibstd::bible
