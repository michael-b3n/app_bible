#include "bibstd/bible/parser_usx.hpp"
#include "bibstd/bible/passage.hpp"
#include "bibstd/bible/passage_info.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/string.hpp"
#include "bibstd/util/timer.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>

namespace bibstd::bible
{
namespace detail
{

///
/// Simple node content walker that concatenates the content of all nodes found,
/// with options to limit the depth and ignore certain node names.
///
class node_content_walker : public pugi::xml_tree_walker
{
public: // Typedefs
  struct rules final
  {
    int max_depth{0};
    std::vector<std::string> ignored_node_names{};
  };

public: // Structors
  node_content_walker(const rules& rules);

public: // Accessors
  ///
  /// Get the content of the nodes found.
  /// \return A string containing the concatenated content of all nodes found.
  ///
  auto content() const -> std::string;

private: // Overrides
  auto for_each(pugi::xml_node& node) -> bool override;

private: // Variables
  const int max_depth_;
  const std::vector<std::string> ignored_node_names_;
  std::string content_;
};

///
///
node_content_walker::node_content_walker(const rules& rules)
  : max_depth_{rules.max_depth}
  , ignored_node_names_{rules.ignored_node_names}
{
}

///
///
auto node_content_walker::content() const -> std::string
{
  return content_;
}

///
///
auto node_content_walker::for_each(pugi::xml_node& node) -> bool
{
  decltype(auto) type = node.type();
  if(type == pugi::xml_node_type::node_pcdata || type == pugi::xml_node_type::node_cdata)
  {
    decltype(auto) parent = node.parent();
    const auto d = depth();
    const auto valid =
      d <= max_depth_ || std::ranges::none_of(ignored_node_names_, [&](const auto& name) { return parent.name() == name; });
    if(valid)
    {
      content_.append(node.value());
    }
  }
  return true;
}

///
/// Tree walker for finding nodes with a certain depth.
/// The walker is initialized with criteria paths, which are paths in the XML tree that specify which nodes to find.
///
class node_depth_finder_walker : public pugi::xml_tree_walker
{
public: // Typedefs
  using result_type = std::map<int, std::vector<pugi::xml_node>>;
  using string_list_type = std::vector<std::string>;

public: // Structors
  ///
  /// Construct a node depth finder walker with the given criteria path.
  /// \param criteria_path The criteria paths to match nodes against. Multiple criteria paths can be provided, the first
  /// matching path will be used. Each criteria path is a string that represents a path in the XML tree, with sections separated
  /// by '/'. The path can contain wildcards ("...").
  ///
  node_depth_finder_walker(const auto& criteria_paths);

public: // Accessors
  ///
  /// Get the found nodes grouped by their depth in the XML tree.
  /// \return A map where the key is the depth and the value is a vector of XML nodes found at that depth.
  ///
  auto found() const -> const result_type&;

private: // Typedefs
  using string_matrix_type = std::vector<string_list_type>;

  struct criteria_data final
  {
    bool starts_with_wildcard = false;
    string_list_type path_sections;
  };

  struct walker_data final
  {
    criteria_data criteria;
    result_type found_nodes;
  };

private: // Constants
  static constexpr std::string_view wildcard = "...";
  static constexpr char section_delimiter = '/';

  static constexpr auto is_wildcard = [](const std::string_view data) { return data == wildcard; };

private: // Implementation
  static auto parse_criteria(const auto& criteria_paths) -> std::vector<walker_data>;
  static auto parse_path_sections(std::string_view criteria_path) -> string_list_type;
  auto matches_criteria(const pugi::xml_node& node, const criteria_data& criteria) const -> bool;

private: // Overrides
  auto for_each(pugi::xml_node& node) -> bool override;
  auto end(pugi::xml_node& node) -> bool override;

private: // Variables
  std::vector<walker_data> data_;
  result_type found_nodes_;
};

///
///
node_depth_finder_walker::node_depth_finder_walker(const auto& criteria_paths)
  : data_{parse_criteria(criteria_paths)}
{
}

///
///
auto node_depth_finder_walker::found() const -> const result_type&
{
  return found_nodes_;
}

///
///
auto node_depth_finder_walker::parse_criteria(const auto& criteria_paths) -> std::vector<walker_data>
{
  auto result = std::vector<walker_data>{};
  std::ranges::for_each(
    criteria_paths,
    [&](const auto& criteria_path)
    {
      const auto sections = bibstd::util::string::split(criteria_path, section_delimiter);
      if(sections.empty())
      {
        THROW_EXCEPTION(std::format("invalid criteria path: reason=\"empty criteria\", path=\"{}\"", criteria_path));
      }
      if(is_wildcard(sections.back()))
      {
        THROW_EXCEPTION(std::format("invalid criteria path: reason=\"cannot end with wildcard\", path=\"{}\"", criteria_path));
      }
      result.emplace_back(criteria_data{is_wildcard(sections.front()), parse_path_sections(criteria_path)});
    }
  );
  return result;
}

///
///
auto node_depth_finder_walker::parse_path_sections(const std::string_view criteria_path) -> string_list_type
{
  auto path_sections = bibstd::util::string::split(criteria_path, wildcard);

  std::ranges::for_each(
    path_sections | std::views::filter([](const auto& section) { return !section.empty(); }),
    [&](auto& element)
    {
      if(util::string::starts_with(element, '/'))
      {
        element = element.substr(1);
      }
      if(util::string::ends_with(element, '/'))
      {
        element.pop_back();
      }
    }
  );
  std::erase_if(path_sections, [](const auto& section) { return section.empty(); });
  return path_sections;
}

///
///
auto node_depth_finder_walker::matches_criteria(const pugi::xml_node& node, const criteria_data& criteria) const -> bool
{
  if(criteria.path_sections.empty())
  {
    return false;
  }
  decltype(auto) path = node.path();
  auto checker = [&path, pos = decltype(std::string::npos){0}](const auto& path_section) mutable
  {
    const auto found_pos = path.find(path_section, pos);
    const auto found = found_pos != std::string::npos;
    if(found)
    {
      pos = found_pos + path_section.size();
    }
    return found;
  };

  decltype(auto) front = criteria.path_sections.front();
  auto result = checker(front);
  if(criteria.starts_with_wildcard)
  {
    result = util::string::starts_with(path, std::format("/{}", front));
  }
  auto rest = criteria.path_sections | std::views::drop(1);
  return result && std::ranges::all_of(rest, [&](const auto& path_section) { return checker(path_section); });
}

///
///
auto node_depth_finder_walker::for_each(pugi::xml_node& node) -> bool
{
  std::ranges::for_each(
    data_ | std::views::filter([&](const auto& element) { return matches_criteria(node, element.criteria); }),
    [&](auto& d) { d.found_nodes[depth()].push_back(node); }
  );
  return true;
}

///
///
auto node_depth_finder_walker::end([[maybe_unused]] pugi::xml_node&) -> bool
{
  std::ranges::for_each(
    data_ | std::views::take_while([&]([[maybe_unused]] const auto&) { return found_nodes_.empty(); }),
    [&](auto& data) { found_nodes_ = std::move(data.found_nodes); }
  );
  return true;
}

///
/// Find the child node specified by the criteria.
/// \param parent The parent node to search within
/// \param criteria_paths Criteria paths to match child nodes, the first matching path will be used.
/// \return The first matching child node, or std::nullopt if not found
///
auto find_highest_child_node(const pugi::xml_node& parent, const auto& criteria_paths) -> std::optional<pugi::xml_node>
{
  pugi::xml_node current = parent;
  auto walker = node_depth_finder_walker{criteria_paths};
  current.traverse(walker);

  auto result = std::optional<pugi::xml_node>{};
  std::ranges::for_each(
    walker.found() | std::views::values | std::views::filter([](const auto& e) { return !e.empty(); }) | std::views::take(1),
    [&](const auto& e) { result = e.front(); }
  );
  return result;
}

///
/// Get all text content from subnodes of the given xml node.
/// \param node The XML node to get content from
/// \return A string containing the concatenated content of all subnodes
///
auto get_all_subnodes_content(const pugi::xml_node& node) -> std::string
{
  pugi::xml_node current = node;
  auto walker = node_content_walker{
    node_content_walker::rules{.max_depth = std::numeric_limits<int>::max(), .ignored_node_names = {}}
  };
  current.traverse(walker);
  return walker.content();
}

///
/// Load a specific entry from the zip reader as a string.
/// \param zip_reader The zip file reader to load from
/// \param entry_name The name of the entry to load
/// \return The loaded entry content as a string, or std::nullopt if not found
///
auto load_entry(const io::zip_file_reader& zip_reader, const std::string& entry_name) -> std::optional<std::string>
{
  using query_flag = io::zip_file_reader::query_flag;
  const auto data = zip_reader.entry(entry_name, query_flag::exclude_directories);
  if(!data)
  {
    LOG_ERROR("failed to load entry: expected \"{}\" file within archive", entry_name);
    return std::nullopt;
  }
  return zip_reader.read_entry_as_string(*data);
}

///
/// Load the scripture name from the XML document.
/// \param doc The XML document to load from
/// \return The loaded scripture name, or std::nullopt if not found
///
auto load_name(const pugi::xml_document& doc) -> std::optional<std::string>
{
  static constexpr auto criteria_paths = std::array{"/.../identification/nameLocal", "/.../identification/name", "/.../name"};
  auto result = std::optional<std::string>{};
  const auto name_node = find_highest_child_node(doc, criteria_paths);
  if(name_node)
  {
    result = get_all_subnodes_content(*name_node);
  }
  else
  {
    LOG_ERROR(
      "expected node in \"metadata.xml\" not found: criteria_paths=\"{}\"", bibstd::util::string::join(criteria_paths, ", ")
    );
  }
  return result;
}

///
/// Load the scripture version from the XML document.
/// \param doc The XML document to load from
/// \return The loaded scripture abbreviation, or std::nullopt if not found
///
auto load_abbreviation(const pugi::xml_document& doc) -> std::optional<std::string>
{
  static constexpr auto criteria_paths =
    std::array{"/.../identification/abbreviationLocal", "/.../identification/abbreviation", "/.../abbreviation"};
  auto result = std::optional<std::string>{};
  const auto abbreviation_node = find_highest_child_node(doc, criteria_paths);
  if(abbreviation_node)
  {
    result = get_all_subnodes_content(*abbreviation_node);
  }
  else
  {
    LOG_ERROR(
      "expected node in \"metadata.xml\" not found: criteria_paths=\"{}\"", bibstd::util::string::join(criteria_paths, ", ")
    );
  }
  return result;
}

///
///
auto load_language(const pugi::xml_document& doc) -> std::optional<std::string>
{
  static constexpr auto criteria_paths = std::array{"/.../language/nameLocal", "/.../language/name", "/.../language"};
  auto result = std::optional<std::string>{};
  const auto language_node = find_highest_child_node(doc, criteria_paths);
  if(language_node)
  {
    result = get_all_subnodes_content(*language_node);
  }
  else
  {
    LOG_ERROR(
      "expected node in \"metadata.xml\" not found: criteria_paths=\"{}\"", bibstd::util::string::join(criteria_paths, ", ")
    );
  }
  return result;
}

///
/// Load copyright information from the XML document.
/// \param doc The XML document to load from
/// \return The loaded copyright information, or std::nullopt if not found
///
auto load_copyright(const pugi::xml_document& doc) -> std::optional<std::string>
{
  static constexpr auto criteria_paths = std::array{"/.../copyright/.../statementContent", "/.../copyright"};
  auto result = std::optional<std::string>{};
  const auto copyright_node = find_highest_child_node(doc, criteria_paths);
  if(copyright_node)
  {
    result = get_all_subnodes_content(*copyright_node);
  }
  else
  {
    LOG_ERROR(
      "expected node in \"metadata.xml\" not found: criteria_paths=\"{}\"", bibstd::util::string::join(criteria_paths, ", ")
    );
  }
  return result;
}

///
/// Load scripture information from the zip reader.
/// \param zip_reader The zip file reader to load from
/// \return The loaded scripture information
///
auto load_info_data(const io::zip_file_reader& zip_reader) -> std::optional<parser_usx::scripture_info>
{
  SCOPED_TIMER_LOG();
  const auto data = load_entry(zip_reader, "metadata.xml");
  if(!data)
  {
    return {};
  }
  pugi::xml_document doc;
  const auto parse_result = doc.load_string(data->c_str());
  if(!parse_result)
  {
    LOG_ERROR("failed to parse \"metadata.xml\": {}", parse_result.description());
    return {};
  }

  return parser_usx::scripture_info{
    .name = load_name(doc).value_or(parser_usx::unknown_name),
    .abbreviation = load_abbreviation(doc).value_or(parser_usx::unknown_abbreviation),
    .language = load_language(doc).value_or(parser_usx::unknown_language),
    .copyright = load_copyright(doc)
  };
}

///
/// Load book data from the zip reader.
/// \param zip_reader The zip file reader to load from
/// \return The loaded book data
///
auto load_book_data(const io::zip_file_reader& zip_reader) -> parser_usx::book_data_type
{
  SCOPED_TIMER_LOG();
  return {};
}

} // namespace detail

///
///
parser_usx::parser_usx(const io::zip_file_reader& zip_reader)
  : info_data_{detail::load_info_data(zip_reader)}
  , book_data_{detail::load_book_data(zip_reader)}
{
  if(valid())
  {
    LOG_INFO(
      "loaded scripture: name=\"{}\", abbreviation=\"{}\", language=\"{}\", copyright=\"{}\"",
      info_data_->name,
      info_data_->abbreviation,
      info_data_->language,
      info_data_->copyright.value_or("not found")
    );
  }
}

///
///
parser_usx::~parser_usx() noexcept = default;

///
///
auto parser_usx::do_valid() const -> bool
{
  return /*!book_data_.empty() &&*/ info_data_.has_value();
}

///
///
auto parser_usx::do_info() const -> scripture_info
{
  static const auto unknown_info =
    scripture_info{.name = unknown_name, .abbreviation = unknown_abbreviation, .language = unknown_language};
  return info_data_.value_or(unknown_info);
}

///
///
auto parser_usx::do_passage_html(const bible::passage_info& info) const -> std::expected<bible::passage_html, error_code>
{
  return std::unexpected(error_code::not_found);
}

} // namespace bibstd::bible
