#include <bibstd/workflow/workflow_settings.hpp>

#include <catch2/catch_test_macros.hpp>

namespace bibstd::workflow
{

TEST_CASE("split_path", "[workflow]")
{
  CHECK(workflow_settings::split_path("").empty());
  CHECK(workflow_settings::split_path("ocr") == std::vector<std::string>{"ocr"});
  CHECK(workflow_settings::split_path("ocr.language") == std::vector<std::string>{"ocr", "language"});
  CHECK(workflow_settings::split_path("ocr.engine.language") == std::vector<std::string>{"ocr", "engine", "language"});
  // Empty segments are left out, so that a malformed path is read like the path it was meant to be
  CHECK(workflow_settings::split_path(".ocr..language.") == std::vector<std::string>{"ocr", "language"});
}

} // namespace bibstd::workflow
