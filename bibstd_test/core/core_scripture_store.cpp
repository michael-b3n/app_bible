#include <bibstd/bible/scripture.hpp>
#include <bibstd/core/core_scripture_store.hpp>

#include <catch2/catch_test_macros.hpp>

#include <format>
#include <ranges>
#include <string>

namespace bibstd::core
{

TEST_CASE("core_scripture_store holds usable scriptures", "[core]")
{
  const core_scripture_store store;
  if(store.scriptures().empty())
  {
    // The scripture archives are not part of the repository, \see bibstd/res/scripture/.gitignore.
    SKIP("no scripture data embedded in this build");
  }
  for(const auto& [name, scripture] : store.scriptures())
  {
    INFO(std::format("scripture: {}", name));
    CHECK(!name.empty());
    REQUIRE(scripture != nullptr);

    // The key is the scripture name, possibly with a " (n)" suffix to disambiguate duplicates.
    CHECK(name.starts_with(scripture->information().name));
    CHECK(!scripture->versification().name().empty());
    CHECK(scripture->versification().count() > 0);
  }
}

TEST_CASE("core_scripture_store keeps scriptures of equal name apart", "[core]")
{
  const core_scripture_store store;
  if(store.scriptures().empty())
  {
    SKIP("no scripture data embedded in this build");
  }

  // The store keys by scripture name, so scriptures sharing one would overwrite each other without
  // the disambiguating suffix. Every loaded scripture has to survive that.
  const auto names = store.scriptures() | std::views::values |
                     std::views::transform([](const auto& scripture) { return scripture->information().name; }) |
                     std::ranges::to<std::vector<std::string>>();
  for(const auto& name : names)
  {
    INFO(std::format("scripture name: {}", name));
    const auto keyed =
      std::ranges::count_if(store.scriptures(), [&](const auto& entry) { return entry.first.starts_with(name); });
    CHECK(keyed == std::ranges::count(names, name));
  }
}

} // namespace bibstd::core
