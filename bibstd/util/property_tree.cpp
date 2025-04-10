#include "util/property_tree.hpp"
#include "util/contains.hpp"
#include "util/exception.hpp"
#include "util/log.hpp"

#include <boost/property_tree/xml_parser.hpp>

#include <algorithm>
#include <fstream>
#include <ranges>

namespace bibstd::util
{

///
///
auto property_tree::create(const std::filesystem::path& tree_file_path) -> property_tree::sptr_type
{
  auto result = property_tree::sptr_type{};
  const auto lock = std::lock_guard(trees_mtx_);
  std::ranges::for_each(
    trees_ | std::views::transform([](const auto t) { return t.lock(); }) | std::views::filter([](const auto t) { return static_cast<bool>(t); }) |
      std::views::filter([&](const auto t) { return t->tree_file_path_ == tree_file_path; }),
    [&](const auto& t) { result = t; });

  if(!result)
  {
    result = std::make_shared<property_tree>(tree_file_path);
    auto it_empty = std::ranges::find_if(trees_, [&](const auto& t) { return !t.lock(); });
    if(it_empty != std::ranges::cend(trees_))
    {
      *it_empty = result;
    }
    else
    {
      trees_.push_back(result);
    }
  }
  return result;
}

///
///
property_tree::property_tree(const std::filesystem::path& tree_file_path)
  : tree_file_path_{tree_file_path}
{
  auto view = trees_ | std::views::transform([](const auto& tree) { return tree.lock(); }) |
              std::views::filter([](const auto& tree) { return static_cast<bool>(tree); });
  if(contains(view, [&](const auto t) { return t->tree_file_path_ == tree_file_path; }))
  {
    THROW_EXCEPTION(exception(std::format("property_tree already existing: file={}", tree_file_path.generic_string())));
  }
  if(tree_file_path.extension() != std::string_view(".xml"))
  {
    THROW_EXCEPTION(exception(std::format("invalid file extension: file={}", tree_file_path.generic_string())));
  }
  if(!std::filesystem::exists(tree_file_path))
  {
    std::filesystem::create_directories(tree_file_path.parent_path());
    if(auto file = std::ofstream{tree_file_path})
    {
      file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
      file.close();
    }
  }
  try
  {
    boost::property_tree::read_xml(tree_file_path_.generic_string(), tree_);
  }
  catch(const boost::property_tree::xml_parser_error& e)
  {
    LOG_ERROR(log_channel, "Failed to read property file: file={}, exception={}", tree_file_path_.generic_string(), e.what());
  }
}

///
///
property_tree::~property_tree() noexcept
{
  const auto lock = std::lock_guard(mtx_);
  try
  {
    boost::property_tree::write_xml(tree_file_path_.generic_string(), tree_);
  }
  catch(const boost::property_tree::xml_parser_error& e)
  {
    LOG_ERROR(log_channel, "Failed to write property file: file={}, exception={}", tree_file_path_.generic_string(), e.what());
  }
}

} // namespace bibstd::util
