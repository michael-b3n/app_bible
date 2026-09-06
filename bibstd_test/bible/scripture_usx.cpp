#include <bibstd/bible/scripture_usx.hpp>
#include <bibstd/io/zip_file_reader.hpp>
#include <bibstd/util/enum.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace bibstd::bible
{
namespace
{

///
/// A scripture shipped with the application, together with the bundle it was loaded from.
///
struct shipped_scripture final
{
  std::string bundle;
  std::unique_ptr<scripture> loaded;
};

///
/// Load every scripture shipped with the application.
/// \return loaded scriptures
///
auto load_shipped_scriptures() -> std::vector<shipped_scripture>
{
  auto result = std::vector<shipped_scripture>{};
  for(const auto& file : std::filesystem::directory_iterator{BIBSTD_TEST_SCRIPTURE_DIR})
  {
    if(file.path().extension() != std::filesystem::path{".zip"})
    {
      continue;
    }
    INFO("bundle: " << file.path().filename().string());
    const auto reader = io::zip_file_reader{file.path()};
    REQUIRE(reader.is_open());
    auto loaded = scripture_usx::create(reader);
    REQUIRE(loaded != nullptr);
    result.emplace_back(file.path().filename().string(), std::move(loaded));
  }
  REQUIRE(!result.empty());
  return result;
}

///
/// Access the scripture shipped in the given bundle.
/// \return the scripture, or nullptr if no such bundle is shipped
///
auto find_bundle(const std::vector<shipped_scripture>& scriptures, const std::string_view bundle)
  -> util::non_owning_ptr<const scripture>
{
  const auto found = std::ranges::find(scriptures, bundle, &shipped_scripture::bundle);
  return found != std::ranges::cend(scriptures) ? found->loaded.get() : nullptr;
}

} // namespace

TEST_CASE("scripture_usx provides names for every book of every shipped scripture", "[bible]")
{
  for(const auto& [bundle, loaded] : load_shipped_scriptures())
  {
    INFO("bundle: " << bundle);
    static constexpr auto books = util::enum_values<book_id>();
    for(const auto book : books)
    {
      INFO("book: " << util::enum_name(book));
      const auto names = loaded->book_information(book);
      REQUIRE(names.has_value());
      // every form is filled from the book's header paragraphs, falling back to the closest alternative
      CHECK(!names->short_name.empty());
      CHECK(!names->abbreviation.empty());
      CHECK(!names->long_name.empty());
    }
  }
}

TEST_CASE("scripture_usx book names are unique within a scripture", "[bible]")
{
  // A collision would mean the parser picked up something other than the book's own header, e.g. content shared
  // between documents.
  for(const auto& [bundle, loaded] : load_shipped_scriptures())
  {
    INFO("bundle: " << bundle);
    static constexpr auto books = util::enum_values<book_id>();
    auto short_names = std::vector<std::string>{};
    for(const auto book : books)
    {
      short_names.push_back(loaded->book_information(book).value().short_name);
    }
    std::ranges::sort(short_names);
    CHECK(std::ranges::adjacent_find(short_names) == std::ranges::cend(short_names));
  }
}

TEST_CASE("scripture_usx book names are read from the book documents", "[bible]")
{
  // The names below are the header paragraphs of the respective USX documents. Reading them there rather than from
  // the bundle metadata keeps them available even for bundles whose metadata lists no names at all.
  const auto scriptures = load_shipped_scriptures();

  SECTION("german scripture")
  {
    const auto* const loaded = find_bundle(scriptures, "text-542b32484b6e38c2-246437.zip");
    REQUIRE(loaded != nullptr);

    const auto genesis = loaded->book_information(book_id::genesis);
    REQUIRE(genesis.has_value());
    CHECK(genesis->short_name == "1. Mose");
    CHECK(genesis->long_name == "Das 1. Buch Mose (Genesis)");

    const auto revelation = loaded->book_information(book_id::revelation);
    REQUIRE(revelation.has_value());
    CHECK(revelation->short_name == "Offenbarung");
    CHECK(revelation->long_name == "Das Buch der Offenbarung Jesu Christi");
  }

  SECTION("english scripture")
  {
    const auto* const loaded = find_bundle(scriptures, "text-de4e12af7f28f599-245514.zip");
    REQUIRE(loaded != nullptr);

    const auto genesis = loaded->book_information(book_id::genesis);
    REQUIRE(genesis.has_value());
    CHECK(genesis->short_name == "Genesis");
    CHECK(genesis->abbreviation == "Gen");
    CHECK(genesis->long_name == "The First Book of Moses, called Genesis");
  }

  SECTION("scripture without running header paragraphs")
  {
    // This bundle ships no "h" paragraph at all, the table of contents entries have to carry the names.
    const auto* const loaded = find_bundle(scriptures, "text-f492a38d0e52db0f-258505.zip");
    REQUIRE(loaded != nullptr);

    const auto song = loaded->book_information(book_id::song_of_solomon);
    REQUIRE(song.has_value());
    CHECK(song->short_name == "Hohelied");
    CHECK(song->abbreviation == "Hld.");
  }
}

TEST_CASE("scripture_usx book names are not the raw identifier", "[bible]")
{
  for(const auto& [bundle, loaded] : load_shipped_scriptures())
  {
    INFO("bundle: " << bundle);
    const auto names = loaded->book_information(book_id::revelation);
    REQUIRE(names.has_value());
    CHECK(names->short_name != util::enum_name(book_id::revelation));
  }
}

TEST_CASE("scripture_usx book names carry no scripture text", "[bible]")
{
  // The header block ends at the first chapter marker. Should the parser run past it, the names would grow into
  // whole verses.
  static constexpr auto max_name_length = 100u;
  for(const auto& [bundle, loaded] : load_shipped_scriptures())
  {
    INFO("bundle: " << bundle);
    static constexpr auto books = util::enum_values<book_id>();
    for(const auto book : books)
    {
      INFO("book: " << util::enum_name(book));
      const auto names = loaded->book_information(book);
      REQUIRE(names.has_value());
      CHECK(names->long_name.size() < max_name_length);
      CHECK(names->short_name.size() <= names->long_name.size());
    }
  }
}

TEST_CASE("scripture_usx reads its information from the bundle root metadata", "[bible]")
{
  // The bundles carry a second, reduced "metadata.xml" inside their "release" directory. Resolving the descriptor
  // against the archive root is what makes the local name and the copyright statement available here.
  for(const auto& [bundle, loaded] : load_shipped_scriptures())
  {
    INFO("bundle: " << bundle);
    const auto info = loaded->information();
    CHECK(info.name != scripture_usx::unknown_name);
    CHECK(info.abbreviation != scripture_usx::unknown_abbreviation);
    CHECK(info.language != scripture_usx::unknown_language);
    REQUIRE(info.copyright.has_value());
    CHECK(!info.copyright->empty());
  }
}

} // namespace bibstd::bible
