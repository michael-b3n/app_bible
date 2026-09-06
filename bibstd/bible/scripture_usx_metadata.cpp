#include "bibstd/bible/scripture_usx_metadata.hpp"
#include "bibstd/bible/scripture_usx.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/string.hpp"
#include "bibstd/util/timer.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace bibstd::bible::usx_metadata
{
namespace
{

///
/// Simple node content walker that concatenates the content of all nodes found.
///
class node_simple_content_walker : public pugi::xml_tree_walker
{
public: // Variables
  std::string content;

public: // Structors
  node_simple_content_walker() = default;

private: // Overrides
  auto for_each(pugi::xml_node& node) -> bool override;
};

///
///
auto node_simple_content_walker::for_each(pugi::xml_node& node) -> bool
{
  decltype(auto) type = node.type();
  if(type == pugi::xml_node_type::node_pcdata || type == pugi::xml_node_type::node_cdata)
  {
    content.append(node.value());
  }
  return true;
}

///
/// Tree walker for finding nodes with a certain depth.
/// The walker is initialized with criteria paths, which are paths in the XML tree that specify which nodes to find.
///
class node_path_finder_walker : public pugi::xml_tree_walker
{
  // Typedefs
  using string_matrix_type = std::vector<std::vector<std::string>>;

  struct criteria_data final
  {
    bool starts_with_wildcard = false;
    typename string_matrix_type::value_type path_sections;
  };

  struct walker_data final
  {
    criteria_data criteria;
    std::map<int, std::vector<pugi::xml_node>> found_nodes;
  };

  // Constants
  static constexpr std::string_view wildcard = "...";
  static constexpr char section_delimiter = '/';
  static constexpr auto is_wildcard = [](const std::string_view data) { return data == wildcard; };

  // Variables
  std::vector<walker_data> data_;
  decltype(walker_data::found_nodes) found_nodes_;

public: // Typedefs
  using result_type = decltype(found_nodes_);
  using string_list_type = typename string_matrix_type::value_type;

public: // Structors
  ///
  /// Construct a node depth finder walker with the given criteria path to match nodes against.
  /// Multiple criteria paths can be provided, the first matching path will be used. Each criteria
  /// path is a string that represents a path in the XML tree, with sections separated by '/'.
  /// The path can contain wildcards ("...").
  ///
  node_path_finder_walker(const auto& criteria_paths);

public: // Accessors
  ///
  /// Get the found nodes grouped by their depth in the XML tree.
  /// \return A map where the key is the depth and the value is a vector of XML nodes found at that depth.
  ///
  auto found() const -> const result_type&;

private: // Implementation
  static auto parse_criteria(const auto& criteria_paths) -> std::vector<walker_data>;
  static auto parse_path_sections(std::string_view criteria_path) -> string_list_type;
  auto matches_criteria(const pugi::xml_node& node, const criteria_data& criteria) const -> bool;

private: // Overrides
  auto for_each(pugi::xml_node& node) -> bool override;
  auto end(pugi::xml_node& node) -> bool override;
};

///
///
node_path_finder_walker::node_path_finder_walker(const auto& criteria_paths)
  : data_{parse_criteria(criteria_paths)}
{
}

///
///
auto node_path_finder_walker::found() const -> const result_type&
{
  return found_nodes_;
}

///
///
auto node_path_finder_walker::parse_criteria(const auto& criteria_paths) -> std::vector<walker_data>
{
  auto result = std::vector<walker_data>{};
  std::ranges::for_each(
    criteria_paths,
    [&](const auto& criteria_path)
    {
      const auto sections = bibstd::util::string::split(criteria_path, section_delimiter);
      if(sections.empty())
      {
        throw util::exception(std::format(R"(invalid criteria path: reason="empty criteria", path="{}")", criteria_path));
      }
      if(is_wildcard(sections.back()))
      {
        throw util::exception(
          std::format(R"(invalid criteria path: reason="cannot end with wildcard", path="{}")", criteria_path)
        );
      }
      result.emplace_back(criteria_data{is_wildcard(sections.front()), parse_path_sections(criteria_path)});
    }
  );
  return result;
}

///
///
auto node_path_finder_walker::parse_path_sections(const std::string_view criteria_path) -> string_list_type
{
  auto path_sections = bibstd::util::string::split(criteria_path, wildcard);

  std::ranges::for_each(
    path_sections | std::views::filter([](const auto& section) { return !section.empty(); }),
    [&](auto& element)
    {
      if(util::string::starts_with(element, section_delimiter))
      {
        element = element.substr(1);
      }
      if(util::string::ends_with(element, section_delimiter))
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
auto node_path_finder_walker::matches_criteria(const pugi::xml_node& node, const criteria_data& criteria) const -> bool
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
    result = util::string::starts_with(path, std::format("{}{}", section_delimiter, front));
  }
  auto rest = criteria.path_sections | std::views::drop(1);
  return result && std::ranges::all_of(rest, [&](const auto& path_section) { return checker(path_section); });
}

///
///
auto node_path_finder_walker::for_each(pugi::xml_node& node) -> bool
{
  std::ranges::for_each(
    data_ | std::views::filter([&](const auto& element) { return matches_criteria(node, element.criteria); }),
    [&](auto& d) { d.found_nodes[depth()].push_back(node); }
  );
  return true;
}

///
///
auto node_path_finder_walker::end([[maybe_unused]] pugi::xml_node& /*node*/) -> bool
{
  std::ranges::for_each(
    data_ | std::views::take_while([&]([[maybe_unused]] const auto&) { return found_nodes_.empty(); }),
    [&](auto& data) { found_nodes_ = std::move(data.found_nodes); }
  );
  return true;
}

///
/// Get all text content from subnodes of the given xml node.
/// \return A string containing the concatenated content of all subnodes
///
auto get_all_subnodes_content(const pugi::xml_node& node) -> std::string
{
  pugi::xml_node current = node;
  auto walker = node_simple_content_walker{};
  current.traverse(walker);
  return walker.content;
}

///
/// Read the text content of the node specified by the criteria that sits highest in the tree.
/// The first matching criteria path will be used.
/// \return Content of the matching node, or std::nullopt if none of the criteria paths matched
///
auto read_node_content(const pugi::xml_document& doc, const auto& criteria_paths) -> std::optional<std::string>
{
  pugi::xml_node current = doc;
  auto walker = node_path_finder_walker{criteria_paths};
  current.traverse(walker);

  auto result = std::optional<std::string>{};
  std::ranges::for_each(
    walker.found() | std::views::values | std::views::filter([](const auto& e) { return !e.empty(); }) | std::views::take(1),
    [&](const auto& e) { result = get_all_subnodes_content(e.front()); }
  );
  if(!result)
  {
    LOG_ERROR(
      "expected node in \"metadata.xml\" not found: criteria_paths=\"{}\"", bibstd::util::string::join(criteria_paths, ", ")
    );
  }
  return result;
}

///
/// Find the archive entry with the given file name that sits closest to the archive root.
/// A bundle may hold the same file name at several depths - "metadata.xml" for instance exists both in the bundle
/// root and inside the "release" directory. Looking the name up directly ignores the directory part and therefore
/// resolves to whichever of them comes first in the archive. Only the root document carries the complete metadata,
/// so the shallowest match is selected.
/// \return The matching entry closest to the archive root, or std::nullopt if not found
///
auto find_root_entry(const io::zip_file_reader& zip_reader, const std::string_view file_name)
  -> std::optional<io::zip_file_reader::zip_entry>
{
  static constexpr auto separator = '/';
  static constexpr auto base_name = [](const std::string_view name)
  {
    const auto pos = name.rfind(separator);
    return pos == std::string_view::npos ? name : name.substr(pos + 1);
  };
  static constexpr auto equal_ignoring_case = [](const std::string_view lhs, const std::string_view rhs)
  {
    static constexpr auto lower = [](const char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); };
    return std::ranges::equal(lhs, rhs, [](const char l, const char r) { return lower(l) == lower(r); });
  };

  const auto entries = zip_reader.entries();
  auto candidates =
    entries |
    std::views::filter([&](const auto& entry) { return entry.name && equal_ignoring_case(base_name(*entry.name), file_name); });

  const auto found =
    std::ranges::min_element(candidates, {}, [](const auto& entry) { return std::ranges::count(*entry.name, separator); });
  if(found == std::ranges::end(candidates))
  {
    LOG_ERROR("failed to load entry: expected \"{}\" file within archive", file_name);
    return std::nullopt;
  }
  return *found;
}

///
/// Load and parse the bundle's root "metadata.xml" document.
/// \return true if the document was loaded and parsed, false otherwise
///
auto load_document(const io::zip_file_reader& zip_reader, pugi::xml_document& doc) -> bool
{
  const auto entry = find_root_entry(zip_reader, "metadata.xml");
  if(!entry)
  {
    return false;
  }
  const auto data = zip_reader.read_entry_as_string(*entry);
  const auto parse_result = doc.load_string(data.c_str());
  if(!parse_result)
  {
    LOG_ERROR("failed to parse \"metadata.xml\": {}", parse_result.description());
    return false;
  }
  return true;
}

} // namespace

///
///
auto parse(const io::zip_file_reader& zip_reader) -> std::optional<scripture::info_type>
{
  SCOPED_TIMER_LOG();
  // clang-format off
  static constexpr auto name_paths = std::array{"/.../identification/nameLocal", "/.../identification/name", "/.../name"};
  static constexpr auto abbreviation_paths = std::array{"/.../identification/abbreviationLocal", "/.../identification/abbreviation", "/.../abbreviation"};
  static constexpr auto language_paths = std::array{"/.../language/nameLocal", "/.../language/name", "/.../language"};
  static constexpr auto copyright_paths = std::array{"/.../copyright/.../statementContent", "/.../copyright"};
  // clang-format on
  pugi::xml_document doc;
  if(!load_document(zip_reader, doc))
  {
    return std::nullopt;
  }
  return scripture::info_type{
    .name = read_node_content(doc, name_paths).value_or(scripture_usx::unknown_name),
    .abbreviation = read_node_content(doc, abbreviation_paths).value_or(scripture_usx::unknown_abbreviation),
    .language = read_node_content(doc, language_paths).value_or(scripture_usx::unknown_language),
    .copyright = read_node_content(doc, copyright_paths)
  };
}

} // namespace bibstd::bible::usx_metadata
