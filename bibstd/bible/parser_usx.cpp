#include "bibstd/bible/parser_usx.hpp"
#include "bibstd/bible/common.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/util/enum.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/string.hpp"
#include "bibstd/util/timer.hpp"
#include "bibstd/util/visit_helper.hpp"

#include <pugixml.hpp>

#ifdef DEBUG
  #include <filesystem>
#endif
#include <algorithm>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <type_traits>

namespace bibstd::bible
{
namespace detail
{

///
/// Node content transform walker that transforms all nodes found from USX to HTML format.
///
class node_usx_to_html_walker : public pugi::xml_tree_walker
{
public: // Constants

public: // Structors
  ///
  /// Construct a USX to HTML walker for a specific book.
  /// \param dest The destination XML node to transform into HTML
  ///
  node_usx_to_html_walker(std::string_view reference_id_prefix);

private: // Typedefs
  using xml_node = pugi::xml_node;
  using xml_attribute = pugi::xml_attribute;

  ///
  /// Data struct to hold state during tree traversal,
  ///
  struct data final
  {
    // Constants
    const std::string reference_id_prefix;

    // Variables
    std::vector<xml_node> to_be_deleted;
    std::vector<xml_node> to_be_unfolded;
    std::string current_chapter;
  };

  ///
  /// Struct to represent an attribute.
  ///
  struct attribute final
  {
    std::string_view name;
    std::optional<std::string_view> value;
    constexpr auto operator==(const attribute&) const -> bool = default;

    ///
    /// Compare this attribute matcher with an actual XML attribute.
    /// \param other The XML attribute to compare against
    /// \return true if the attribute name matches and value matches (if value is specified), false otherwise
    ///
    auto operator==(const pugi::xml_attribute& other) const -> bool;
  };

  ///
  /// Struct to represent a node.
  ///
  struct node final
  {
    std::string_view name;
    std::optional<attribute> attr;

    constexpr auto operator==(const node&) const -> bool = default;

    ///
    /// Compare this node matcher with an actual XML node.
    /// \param other The XML node to compare against
    /// \return true if the node name and attribute (if specified) match, false otherwise
    ///
    auto operator==(const pugi::xml_node& other) const -> bool;
  };

  ///
  /// Function pointer type for node transformation actions.
  ///
  using a_fptr = void (*)(xml_node&, data&);

  ///
  /// Struct to represent a node rename action.
  ///
  struct a_rename final
  {
    std::string_view new_name;
    auto operator==(const a_rename&) const -> bool = default;
  };

private: // Constants
  static constexpr auto _any_attr = std::optional<attribute>{};
  static constexpr auto _any_value = std::optional<std::string_view>{};

  ///
  /// Function pointer marking a node for deletion.
  ///
  static constexpr a_fptr a_delete = [](xml_node& node, data& d) { d.to_be_deleted.push_back(node); };

  ///
  /// Function pointer unfolding a node for unfolding.
  ///
  static constexpr a_fptr a_unfold = [](xml_node& node, data& d) { d.to_be_unfolded.push_back(node); };

  ///
  /// Function pointer caching chapter and deleting the node.
  ///
  static constexpr a_fptr a_cache_chapter_and_delete = [](xml_node& node, data& d)
  {
    const auto chapter_number = std::string_view(node.attribute("number").value());
    d.current_chapter = chapter_number;
    node.text().set(std::format("{}", chapter_number));
    node.remove_attributes();
    node.set_name(parser::format_chapter_number);
  };

  ///
  /// Function pointer inserting passage number and renaming the node to passage number format.
  ///
  static constexpr a_fptr a_insert_passage_number_and_rename = [](xml_node& node, data& d)
  {
    const auto verse_number = std::string_view(node.attribute("number").value());
    node.text().set(std::format("{}", verse_number));
    node.remove_attributes();
    const auto id = std::format(parser_usx::template_reference_id, d.reference_id_prefix, d.current_chapter, verse_number);
    node.append_attribute(parser_usx::attribute_reference_id).set_value(id);
    node.set_name(parser::format_verse_number);
  };

  ///
  /// Mapping of nodes to their corresponding transformation actions (rename, delete, unfold, etc.).
  ///
  static constexpr auto node_transform_map = util::make_const_map<node, std::variant<a_rename, a_fptr>>({
    {                          node{"usx", _any_attr},                                     a_unfold},
    {                         node{"book", _any_attr},                                     a_delete},
    {        node{"para", attribute{"style", "toc1"}},                                     a_delete},
    {           node{"para", attribute{"style", "p"}},           a_rename{parser::format_paragraph}},
    {                         node{"para", _any_attr},                                     a_delete},
    {node{"chapter", attribute{"number", _any_value}},                   a_cache_chapter_and_delete},
    {                      node{"chapter", _any_attr},                                     a_delete},
    {  node{"verse", attribute{"number", _any_value}},           a_insert_passage_number_and_rename},
    {                        node{"verse", _any_attr},                                     a_delete},
    {          node{"char", attribute{"style", "bd"}},                  a_rename{parser::html_bold}},
    {          node{"char", attribute{"style", "it"}},                a_rename{parser::html_italic}},
    {          node{"char", attribute{"style", "em"}},          a_rename{parser::format_emphasized}},
    {          node{"char", attribute{"style", "nd"}},         a_rename{parser::format_name_of_god}},
    {         node{"char", attribute{"style", "add"}}, a_rename{parser::format_translator_addition}},
    {                         node{"char", _any_attr},                                     a_unfold},
    {                         node{"note", _any_attr},                                     a_delete}
  });

private: // Overrides
  ///
  /// Process each node during tree traversal.
  /// Transforms USX nodes to HTML format by renaming nodes, caching chapter numbers,
  /// inserting verse numbers, and marking nodes for deletion.
  /// \param node The current XML node being traversed
  /// \return true to continue traversal, false to stop
  ///
  auto for_each(xml_node& node) -> bool override;

  ///
  /// Called when traversal exits a node and all its children have been processed.
  /// First unfolds nodes by moving their children to their parent (lifting content up),
  /// then safely removes all nodes that were marked for deletion during traversal.
  /// Processes nodes in reverse order with parent validation to avoid double-deletion.
  /// \param node The node being exited (unused)
  /// \return true to continue traversal
  ///
  auto end(xml_node& node) -> bool override;

private: // Variables
  data d_;
};

///
///
auto node_usx_to_html_walker::attribute::operator==(const xml_attribute& other) const -> bool
{
  if(other && name == std::string_view(other.name()))
  {
    return !value.has_value() || *value == std::string_view{other.value()};
  }
  return false;
}

///
///
auto node_usx_to_html_walker::node::operator==(const xml_node& other) const -> bool
{
  if(name == std::string_view(other.name()))
  {
    return !attr.has_value() || *attr == other.attribute(attr->name);
  }
  return false;
}

///
///
node_usx_to_html_walker::node_usx_to_html_walker(std::string_view reference_id_prefix)
  : d_{.reference_id_prefix{reference_id_prefix}, .to_be_deleted{}, .to_be_unfolded{}, .current_chapter{}}
{
}

///
///
auto node_usx_to_html_walker::for_each(xml_node& node) -> bool
{
  decltype(auto) type = node.type();
  if(type == pugi::xml_node_type::node_element)
  {
    const auto it = std::ranges::find_if(node_transform_map, [&](const auto& element) { return element.first == node; });
    if(it != std::ranges::cend(node_transform_map))
    {
      decltype(auto) to_node = it->second;
      util::visit_lambdas(
        to_node,
        [&](const a_rename& rename)
        {
          node.set_name(rename.new_name);
          node.remove_attributes();
        },
        [&](const a_fptr& action) { action(node, d_); }
      );
    }
    else
    {
      LOG_WARN(
        "no transformation specified for node: name=\"{}\", attributes=[{}]",
        node.name(),
        util::string::join(
          node.attributes() |
            std::views::transform([](const auto& attr) { return std::format("{}=\"{}\"", attr.name(), attr.value()); }),
          ", "
        )
      );
    }
  }
  return true;
}

///
///
auto node_usx_to_html_walker::end([[maybe_unused]] xml_node& node) -> bool
{
  // First pass: unfold nodes by moving their content to parent
  std::ranges::for_each(
    d_.to_be_unfolded | std::views::reverse,
    [](auto& node_to_unfold)
    {
      if(auto parent = node_to_unfold.parent())
      {
        while(auto child = node_to_unfold.first_child())
        {
          parent.insert_move_before(child, node_to_unfold);
        }
        parent.remove_child(node_to_unfold);
      }
    }
  );

  // Second pass: remove all nodes marked for deletion
  std::ranges::for_each(
    d_.to_be_deleted | std::views::reverse,
    [](auto& node_to_remove)
    {
      if(auto parent = node_to_remove.parent())
      {
        parent.remove_child(node_to_remove);
      }
    }
  );
  return true;
}

///
/// Simple node content walker that concatenates the content of all nodes found.
///
class node_simple_content_walker : public pugi::xml_tree_walker
{
public: // Structors
  node_simple_content_walker() = default;

public: // Variables
  std::string content;

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
  node_path_finder_walker(const auto& criteria_paths);

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
        throw util::exception(std::format("invalid criteria path: reason=\"empty criteria\", path=\"{}\"", criteria_path));
      }
      if(is_wildcard(sections.back()))
      {
        throw util::exception(
          std::format("invalid criteria path: reason=\"cannot end with wildcard\", path=\"{}\"", criteria_path)
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
auto node_path_finder_walker::end([[maybe_unused]] pugi::xml_node&) -> bool
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
  auto walker = node_path_finder_walker{criteria_paths};
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
  auto walker = node_simple_content_walker{};
  current.traverse(walker);
  return walker.content;
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
  const auto data = zip_reader.entry(entry_name, {query_flag::exclude_directories, query_flag::case_insensitive});
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
/// Load the scripture language from the XML document.
/// \param doc The XML document to load from
/// \return The loaded scripture language, or std::nullopt if not found
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
/// Convert USX XML nodes to HTML syntax.
/// \param id The book ID corresponding to the USX content being converted
/// \param src The source XML node containing USX content to convert
/// \param dest The destination XML node to append the converted HTML content to
/// \return true if conversion was successful, false otherwise
///
auto usx_to_html(book_id id, const pugi::xml_node& src, pugi::xml_node& dest) -> bool
{
  try
  {
    const auto valid = static_cast<bool>(src) && static_cast<bool>(dest);
    if(valid)
    {
      auto usx_node = src.child("usx");
      usx_node.set_name(util::enum_name(id));
      usx_node.remove_attributes();
      auto walker = node_usx_to_html_walker{util::enum_name(id)};
      usx_node.traverse(walker);
      dest.append_copy(usx_node);
      return true;
    }
    else
    {
      LOG_ERROR("invalid nodes provided for usx to html conversion");
    }
    return valid;
  }
  catch(const util::exception& e)
  {
    LOG_ERROR("failed to transform usx to html: {}", e.what());
    return false;
  }
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
auto load_book_data(const io::zip_file_reader& zip_reader) -> std::unique_ptr<pugi::xml_document>
{
  SCOPED_TIMER_LOG();
  auto doc = std::make_unique<pugi::xml_document>();
  pugi::xml_document src;

  const auto success = std::ranges::all_of(
    parser_usx::books,
    [&](const auto& book)
    {
      const auto& [id, abbreviation] = book;
      src.reset();
      const auto content = load_entry(zip_reader, std::format("{}.usx", abbreviation));
      if(!content.has_value() || content->empty())
      {
        LOG_ERROR("failed to load \"{}\" data: expected \"{}.usx\" file within archive", util::enum_name(id), abbreviation);
        return false;
      }
      const auto parse_result = src.load_string(content->c_str());
      if(!parse_result)
      {
        LOG_ERROR("failed to parse \"{}.usx\": {}", abbreviation, parse_result.description());
        return false;
      }
      return usx_to_html(id, src, *doc);
    }
  );
  if(!success)
  {
    doc->reset();
  }
#ifdef DEBUG
  std::filesystem::create_directory("debug");
  doc->save_file("debug/debug_output.xml");
#endif
  return doc;
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
  return book_data_ && info_data_.has_value();
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
auto parser_usx::do_passage_html(const reference& ref) const -> std::expected<html_passage, error_code>
{
  struct xml_string_writer final : public pugi::xml_writer
  {
    // Overrides
    auto write(const void* data, size_t size) -> void override { out.append(static_cast<const char*>(data), size); }

    // Variables
    std::string out;
  };

  const auto reference_id =
    std::format(parser_usx::template_reference_id, util::enum_name(ref.book()), ref.chapter(), ref.verse());
  decltype(auto) node =
    book_data_->find_node([&](pugi::xml_node n)
                          { return reference_id == n.attribute(parser_usx::attribute_reference_id).as_string(); });
  auto writer = xml_string_writer{};
  node.parent().print(writer, "", pugi::format_raw);

  const auto reference_pos = writer.out.find(reference_id);
  const auto offset = reference_id.size() + 2 /*'"' + '>'*/;
  if(reference_pos != std::string::npos && reference_pos + offset < writer.out.size())
  {
    // return reference_pos + offset as well to get the position of the actual verse content
    // within the generated HTML, which can be used for highlighting the verse
    return html_passage{reference_pos + offset, std::move(writer.out)};
  }
  else
  {
    LOG_ERROR("failed to find reference id in generated html: reference_id=\"{}\"", reference_id);
    return std::unexpected(error_code::not_found);
  }
}

} // namespace bibstd::bible
