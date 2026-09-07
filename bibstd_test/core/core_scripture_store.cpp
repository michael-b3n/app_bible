#include <bibstd/bible/scripture.hpp>
#include <bibstd/core/core_scripture_store.hpp>

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <ranges>
#include <string>

namespace bibstd::core
{
namespace
{

///
/// Number of scripture bundles shipped with the application.
/// \return count of zip bundles in the scripture resource folder
///
auto shipped_bundle_count() -> std::size_t
{
  return std::ranges::count_if(
    std::filesystem::directory_iterator{BIBSTD_TEST_SCRIPTURE_DIR},
    [](const auto& entry) { return entry.path().extension() == std::filesystem::path{".zip"}; }
  );
}

} // namespace

TEST_CASE("core_scripture_store loads every shipped scripture", "[core]")
{
  const core_scripture_store store;
  CHECK(store.scriptures().size() == shipped_bundle_count());
}

TEST_CASE("core_scripture_store holds usable scriptures", "[core]")
{
  const core_scripture_store store;
  REQUIRE(!store.scriptures().empty());
  for(const auto& [name, scripture] : store.scriptures())
  {
    INFO("scripture: " << name);
    CHECK(!name.empty());
    REQUIRE(scripture != nullptr);

    // The key is the scripture name, possibly with a " (n)" suffix to disambiguate duplicates.
    CHECK(name.starts_with(scripture->information().name));
    CHECK(!scripture->versification().name().empty());
    CHECK(scripture->versification().count() > 0);
  }
}

TEST_CASE("core_scripture_store keys the scriptures by a unique name", "[core]")
{
  const core_scripture_store store;
  const auto names = store.scriptures() | std::views::keys | std::ranges::to<std::vector<std::string>>();
  CHECK(std::ranges::adjacent_find(names) == std::ranges::cend(names));
}

} // namespace bibstd::core
