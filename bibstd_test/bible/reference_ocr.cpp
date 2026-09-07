#include "ocr_capture_data.hpp"

#include <bibstd/bible/reference_ocr.hpp>
#include <bibstd/data/pixel.hpp>
#include <bibstd/data/plane.hpp>
#include <bibstd/math/coordinates.hpp>
#include <bibstd/math/rect.hpp>
#include <bibstd/math/value_range.hpp>
#include <bibstd/txt/ocr_engine.hpp>
#include <bibstd/util/screen_types.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
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
auto to_rect(const test::capture_box& box) -> util::screen_rect_type
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
/// OCR engine that replays captured data instead of looking at the image. Recognition honours the
/// subarea the same way a real engine does: only overlapping words are reported and their boxes are
/// relative to the subarea, so that reference_ocr has to shift them back itself.
///
class capture_engine final : public txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>
{
  // Variables
  test::capture_data data_;
  name_type name_;
  std::optional<area_type> subarea_;

public: // Constants
  static constexpr auto default_name = "capture";

public: // Structors
  explicit capture_engine(test::capture_data data, name_type name = default_name)
    : data_{std::move(data)}
    , name_{std::move(name)}
  {
  }

public: // Accessors
  ///
  /// \return Subarea the engine was initialized with last
  ///
  auto last_subarea() const -> const std::optional<area_type>& { return subarea_; }

public: // Overrides
  auto name() const -> name_type override { return name_; }

  auto initialize([[maybe_unused]] pixel_plane_view_type image, std::optional<area_type> subarea) -> void override
  {
    subarea_ = subarea;
  }

  auto recognize() const -> recognition_data override
  {
    const auto offset = subarea_ ? util::screen_coordinates_type{
                                     -static_cast<util::screen_rect_type::value_type>(subarea_->origin().x()),
                                     -static_cast<util::screen_rect_type::value_type>(subarea_->origin().y())
                                   }
                                 : util::screen_coordinates_type{0, 0};
    const auto to_word = [&](const test::capture_text& t) { return word{t.text, shifted(to_rect(t.box), offset)}; };
    const auto to_line = [&](const test::capture_text& t) { return line{t.text, shifted(to_rect(t.box), offset)}; };
    const auto to_paragraph = [&](const test::capture_text& t) { return paragraph{t.text, shifted(to_rect(t.box), offset)}; };

    auto result = recognition_data{};
    for(const auto& element : data_.words)
    {
      if(subarea_ && !math::overlap(*subarea_, area_type{to_rect(element.word.box)}))
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
};

///
/// OCR engine without layout analysis support, to reach the paths that require one.
///
class plain_engine final : public txt::ocr_engine<txt::ocr_engine_tag_plain>
{
public: // Constants
  static constexpr auto default_name = "plain";

public: // Overrides
  auto name() const -> name_type override { return default_name; }

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
auto designed_capture() -> test::capture_data
{
  const auto line1 = test::capture_text{
    .text = "Der Vers Johannes 3,\n", .box = {.x = 50, .y = 50, .width = 200, .height = 20}
  };
  const auto line2 = test::capture_text{
    .text = "16 ist bekannt.\n", .box = {.x = 50, .y = 80, .width = 200, .height = 20}
  };
  const auto line3 = test::capture_text{
    .text = "Ein anderer Absatz.\n", .box = {.x = 50, .y = 120, .width = 200, .height = 20}
  };
  const auto paragraph1 = test::capture_text{
    .text = line1.text + line2.text, .box = {.x = 50, .y = 50, .width = 200, .height = 50}
  };
  const auto paragraph2 = test::capture_text{.text = line3.text, .box = line3.box};

  const auto word = [](const std::string& text, const std::int32_t x, const std::int32_t y, const std::int32_t width)
  {
    return test::capture_text{
      .text = text, .box = {.x = x, .y = y, .width = width, .height = 20}
    };
  };

  return test::capture_data{
    .id = "designed",
    .width = 300,
    .height = 180,
    .layouts =
      {test::capture_layout{.line = line1.box, .paragraph = paragraph1.box},
                test::capture_layout{.line = line2.box, .paragraph = paragraph1.box},
                test::capture_layout{.line = line3.box, .paragraph = paragraph2.box}},
    .words = {
                test::capture_word{.word = word("Der", 50, 50, 30), .line = line1, .paragraph = paragraph1},
                test::capture_word{.word = word("Vers", 90, 50, 40), .line = line1, .paragraph = paragraph1},
                test::capture_word{.word = word("Johannes", 140, 50, 80), .line = line1, .paragraph = paragraph1},
                test::capture_word{.word = word("3,", 230, 50, 20), .line = line1, .paragraph = paragraph1},
                test::capture_word{.word = word("16", 50, 80, 20), .line = line2, .paragraph = paragraph1},
                test::capture_word{.word = word("ist", 80, 80, 30), .line = line2, .paragraph = paragraph1},
                test::capture_word{.word = word("bekannt.", 120, 80, 80), .line = line2, .paragraph = paragraph1},
                test::capture_word{.word = word("Ein", 50, 120, 30), .line = line3, .paragraph = paragraph2},
                test::capture_word{.word = word("anderer", 90, 120, 70), .line = line3, .paragraph = paragraph2},
                test::capture_word{.word = word("Absatz.", 170, 120, 70), .line = line3, .paragraph = paragraph2}
    }
  };
}

///
/// \return Algorithm data driving both algorithms with the capture engine
///
auto algorithm_data(const reference_ocr::algorithm_type algorithm) -> reference_ocr::algorithm_data
{
  return reference_ocr::algorithm_data{
    .algorithm = algorithm,
    .engine_name_character_recognition = capture_engine::default_name,
    .engine_name_layout_recognition = capture_engine::default_name
  };
}

///
/// \return Centre of a capture box, the position a user would point at to hit the word
///
auto centre(const test::capture_box& box) -> reference_ocr::position_type
{
  return math::coordinates(box.x + (box.width / 2), box.y + (box.height / 2));
}

///
/// The image is only passed through to the engines, so an empty one is enough for every test here.
///
const auto no_image = util::pixel_plane_type{};

} // namespace

TEST_CASE("reference_ocr resolves the position to a character of the paragraph", "[bible]")
{
  const auto capture = designed_capture();
  auto engines = reference_ocr::ocr_engine_list_type{};
  engines.emplace_back(std::make_unique<capture_engine>(capture));

  GIVEN("a position on a word of the first paragraph")
  {
    const auto& johannes = capture.words.at(2);
    const auto result = reference_ocr::run(
      engines,
      util::pixel_plane_view_type{no_image},
      centre(johannes.word.box),
      algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition)
    );
    REQUIRE(result.has_value());

    // Both lines belong to one paragraph, so the whole paragraph is returned, line break included.
    CHECK(result->text == "Der Vers Johannes 3,\n16 ist bekannt.\n");
    CHECK(result->character_bounding_boxes.size() == result->text.size());

    // The cursor character has to land inside the word that was pointed at.
    const auto word_begin = result->text.find("Johannes");
    REQUIRE(word_begin != std::string::npos);
    CHECK(result->cursor_character_index >= word_begin);
    CHECK(result->cursor_character_index < word_begin + std::string_view{"Johannes"}.size());
  }
  GIVEN("a position on a word of the second paragraph")
  {
    const auto& absatz = capture.words.at(9);
    const auto result = reference_ocr::run(
      engines,
      util::pixel_plane_view_type{no_image},
      centre(absatz.word.box),
      algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition)
    );
    REQUIRE(result.has_value());
    CHECK(result->text == "Ein anderer Absatz.\n");
  }
  GIVEN("a position outside every line")
  {
    const auto result = reference_ocr::run(
      engines,
      util::pixel_plane_view_type{no_image},
      math::coordinates(290, 175),
      algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition)
    );
    REQUIRE(result.has_value());
    CHECK(result->text.empty());
    CHECK(result->character_bounding_boxes.empty());
  }
}

TEST_CASE("reference_ocr widens the recognition area to the paragraph", "[bible]")
{
  const auto capture = designed_capture();
  auto engine = std::make_unique<capture_engine>(capture);
  const auto* const engine_ptr = engine.get();
  auto engines = reference_ocr::ocr_engine_list_type{};
  engines.emplace_back(std::move(engine));

  const auto run_at = [&](const test::capture_box& box)
  {
    return reference_ocr::run(
      engines,
      util::pixel_plane_view_type{no_image},
      centre(box),
      algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition)
    );
  };

  GIVEN("a position on the first line of a two line paragraph")
  {
    std::ignore = run_at(capture.words.at(0).word.box);
    REQUIRE(engine_ptr->last_subarea().has_value());
    const auto& area = *engine_ptr->last_subarea();

    // The next line of the same paragraph is taken in, then half a line height is added as padding.
    CHECK(area.origin().x() == 40);
    CHECK(area.origin().y() == 40);
    CHECK(math::size(area.horizontal_range()) == 220);
    CHECK(math::size(area.vertical_range()) == 70);
  }
  GIVEN("a position on a paragraph of a single line")
  {
    std::ignore = run_at(capture.words.at(7).word.box);
    REQUIRE(engine_ptr->last_subarea().has_value());
    const auto& area = *engine_ptr->last_subarea();

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
  const auto capture = designed_capture();
  auto engines = reference_ocr::ocr_engine_list_type{};
  engines.emplace_back(std::make_unique<capture_engine>(capture));

  const auto& johannes = capture.words.at(2);
  const auto result = reference_ocr::run(
    engines,
    util::pixel_plane_view_type{no_image},
    centre(johannes.word.box),
    algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition)
  );
  REQUIRE(result.has_value());
  REQUIRE(result->cursor_character_index < result->character_bounding_boxes.size());

  const auto& cursor_box = result->character_bounding_boxes.at(result->cursor_character_index);
  REQUIRE(cursor_box.has_value());
  CHECK(math::overlap(*cursor_box, to_rect(johannes.word.box)).has_value());
}

TEST_CASE("reference_ocr recognizes without layout analysis", "[bible]")
{
  const auto capture = designed_capture();
  auto engines = reference_ocr::ocr_engine_list_type{};
  engines.emplace_back(std::make_unique<capture_engine>(capture));

  const auto result = reference_ocr::run(
    engines,
    util::pixel_plane_view_type{no_image},
    centre(capture.words.at(2).word.box),
    algorithm_data(reference_ocr::algorithm_type::recognize_just_with_line_recognition)
  );
  REQUIRE(result.has_value());

  // Without a subarea the whole image is recognized, but the reported element is the same.
  CHECK(result->text == "Der Vers Johannes 3,\n16 ist bekannt.\n");
  CHECK(result->character_bounding_boxes.size() == result->text.size());
}

TEST_CASE("reference_ocr rejects unusable engine setups", "[bible]")
{
  const auto capture = designed_capture();
  const auto position = centre(capture.words.at(2).word.box);
  const auto image = util::pixel_plane_view_type{no_image};

  GIVEN("no engine with the requested character recognition name")
  {
    auto engines = reference_ocr::ocr_engine_list_type{};
    engines.emplace_back(std::make_unique<capture_engine>(capture));
    auto ad = algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
    ad.engine_name_character_recognition = "not the name of any engine";

    const auto result = reference_ocr::run(engines, image, position, ad);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == reference_ocr::unexpected_ocr_result::error);
  }
  GIVEN("no engine for the layout recognition")
  {
    auto engines = reference_ocr::ocr_engine_list_type{};
    engines.emplace_back(std::make_unique<capture_engine>(capture));
    auto ad = algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
    ad.engine_name_layout_recognition = std::nullopt;

    const auto result = reference_ocr::run(engines, image, position, ad);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == reference_ocr::unexpected_ocr_result::error);
  }
  GIVEN("an engine without layout analysis support for the paragraph recognition")
  {
    auto engines = reference_ocr::ocr_engine_list_type{};
    engines.emplace_back(std::make_unique<capture_engine>(capture));
    engines.emplace_back(std::make_unique<plain_engine>());
    auto ad = algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
    ad.engine_name_layout_recognition = plain_engine::default_name;

    const auto result = reference_ocr::run(engines, image, position, ad);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == reference_ocr::unexpected_ocr_result::error);
  }
  GIVEN("an undefined engine")
  {
    auto engines = reference_ocr::ocr_engine_list_type{};
    engines.emplace_back(std::monostate{});
    auto ad = algorithm_data(reference_ocr::algorithm_type::recognize_just_with_line_recognition);
    ad.engine_name_character_recognition = "Undefined";

    const auto result = reference_ocr::run(engines, image, position, ad);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == reference_ocr::unexpected_ocr_result::error);
  }
  GIVEN("an unsupported algorithm")
  {
    auto engines = reference_ocr::ocr_engine_list_type{};
    engines.emplace_back(std::make_unique<capture_engine>(capture));
    auto ad = algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition);
    ad.algorithm = static_cast<reference_ocr::algorithm_type>(-1);

    const auto result = reference_ocr::run(engines, image, position, ad);
    REQUIRE_FALSE(result.has_value());
    CHECK(result.error() == reference_ocr::unexpected_ocr_result::unsupported);
  }
}

TEST_CASE("reference_ocr handles captured screenshots", "[bible]")
{
  // The captures are produced from local screenshots and are not part of the repository,
  // \see bibstd_test/res/ocr/README.md.
  const auto captures = test::read_captures(std::filesystem::path{BIBSTD_TEST_OCR_DIR});
  if(captures.empty())
  {
    SKIP("no ocr captures in " << BIBSTD_TEST_OCR_DIR);
  }

  for(const auto& capture : captures)
  {
    INFO("capture: " << capture.id);
    const auto image_area = util::screen_rect_type{math::coordinates(0, 0), capture.width, capture.height};

    for(const auto& element : capture.words)
    {
      INFO("word: \"" << element.word.text << "\"");
      auto engines = reference_ocr::ocr_engine_list_type{};
      engines.emplace_back(std::make_unique<capture_engine>(capture));

      const auto result = reference_ocr::run(
        engines,
        util::pixel_plane_view_type{no_image},
        centre(element.word.box),
        algorithm_data(reference_ocr::algorithm_type::recognize_with_paragraph_recognition)
      );
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
      if(cursor_box)
      {
        CHECK(math::overlap(*cursor_box, to_rect(element.word.box)).has_value());
      }
    }
  }
}

} // namespace bibstd::bible
