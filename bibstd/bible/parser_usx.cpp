#include "bibstd/bible/parser_usx.hpp"
#include "bibstd/bible/common.hpp"
#include "bibstd/io/zip_file_reader.hpp"
#include "bibstd/util/contains.hpp"
#include "bibstd/util/enum.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/string.hpp"
#include "bibstd/util/timer.hpp"
#include "bibstd/util/uid.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <array>
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
/// Serialize inline USX content to HTML string.
/// Handles char styles (bold, italic, etc.) and skips notes.
/// \param node The XML node to serialize
/// \return HTML string
///
auto serialize_inline_to_html(const pugi::xml_node& node) -> std::string
{
  auto result = std::string{};
  for(auto child : node.children())
  {
    const auto type = child.type();
    if(type == pugi::node_pcdata || type == pugi::node_cdata)
    {
      result.append(child.value());
    }
    else if(type == pugi::node_element)
    {
      const auto name = std::string_view{child.name()};
      if(name == "char")
      {
        const auto style = std::string_view{child.attribute("style").value()};
        const auto inner = serialize_inline_to_html(child);
        // clang-format off
        if(style == "bd") { result.append(std::format("<{0}>{1}</{0}>", parser::html_bold, inner)); }
        else if(style == "it" || style == "em") { result.append(std::format("<{0}>{1}</{0}>", parser::html_italic, inner)); }
        else if(style == "nd") { result.append(std::format("<{0}>{1}</{0}>", parser::format_name_of_god, inner)); }
        else if(style == "add") { result.append(std::format("<{0}>{1}</{0}>", parser::format_translator_addition, inner)); }
        else { result.append(inner); }
        // clang-format on
      }
      else if(name == "note")
      {
        // skip footnotes
      }
      else
      {
        result.append(serialize_inline_to_html(child));
      }
    }
  }
  return result;
}

///
/// Check if a paragraph style is a content paragraph (not header/title/table of contents).
/// \param style The style attribute value
/// \return true if it is a content paragraph
///
auto is_content_paragraph(std::string_view style) -> bool
{
  static constexpr auto non_content = std::array{"h", "toc1", "toc2", "toc3", "mt", "mt1", "mt2", "mt3"};
  return !util::contains(non_content, style);
}

///
/// Extract text content from an XML node tree.
/// \param node The XML node
/// \return concatenated text content
///
auto get_inline_text(const pugi::xml_node& node) -> std::string
{
  auto result = std::string{};
  for(auto child : node.children())
  {
    if(child.type() == pugi::node_pcdata || child.type() == pugi::node_cdata)
    {
      result.append(child.value());
    }
    else if(child.type() == pugi::node_element)
    {
      result.append(get_inline_text(child));
    }
  }
  return result;
}

///
/// Parsing state for tracking the current verse being assembled.
///
struct verse_parse_state final
{
  // Typedefs
  using paragraph_id_type = util::uid<struct paragraph_id_tag>;

  struct segment final
  {
    paragraph_id_type paragraph_id{};
    std::string_view paragraph_attribute_value{parser::html_custom_undefined};
    std::string content{""};
  };

  std::vector<segment> segments{};
  std::vector<std::string> current_xrefs{};
  std::optional<std::uint32_t> chapter{};
  std::optional<std::uint32_t> verse{};
  std::optional<paragraph_id_type> current_paragraph_id{};
};

///
/// Flush accumulated segments and cross-references for the current verse into the passage map.
/// Resets segments and cross-references afterwards. Does nothing if no complete verse is pending.
/// \param id The book identifier used to create the reference
/// \param state The current parsing state (segments and xrefs are cleared)
/// \param passage_map The map to store the assembled passage into
///
auto flush_verse(const book_id id, verse_parse_state& state, parser_usx::passage_map_type& passage_map) -> void
{
  if(!state.verse || !state.chapter || state.segments.empty())
  {
    return;
  }
  std::erase_if(state.segments, [](const auto& s) { return s.content.empty(); });
  if(state.segments.empty())
  {
    return;
  }

  auto html = std::string{};
  for(const auto& seg : state.segments)
  {
    html.append(std::format(R"(<p {}="{}">)", parser::html_custom_attr_id, seg.paragraph_attribute_value));
    html.append(seg.content);
    html.append("</p>");
  }
  const auto ref = reference::create(id, *state.chapter, *state.verse);
  if(!ref)
  {
    LOG_WARN("invalid reference: book={}, chapter={}, verse={}", util::enum_name(id), *state.chapter, *state.verse);
    state.segments.clear();
    state.current_xrefs.clear();
    return;
  }
  passage_map[*ref] = {std::move(html), std::move(state.current_xrefs)};
  state.current_xrefs = {};
  state.segments.clear();
}

///
/// Handle a chapter marker: flush the current verse and begin a new chapter.
/// \param id The book identifier
/// \param chapter_number The new chapter number
/// \param state The current parsing state (chapter set, verse reset)
/// \param passage_map The map to flush the previous verse into
///
auto begin_chapter(
  const book_id id, const std::uint32_t chapter_number, verse_parse_state& state, parser_usx::passage_map_type& passage_map
) -> void
{
  flush_verse(id, state, passage_map);
  state.chapter = chapter_number;
  state.verse = std::nullopt;
}

///
/// Handle a verse marker: flush the current verse and begin a new one.
/// \param id The book identifier
/// \param verse_number The new verse number
/// \param state The current parsing state (verse number set)
/// \param passage_map The map to flush the previous verse into
///
auto begin_verse(
  const book_id id, const std::uint32_t verse_number, verse_parse_state& state, parser_usx::passage_map_type& passage_map
) -> void
{
  flush_verse(id, state, passage_map);
  state.verse = verse_number;
}

///
/// Append text content to the current verse's active segment.
/// Creates a new segment when the paragraph context changes. Ignores content before any verse starts.
/// \param state The current parsing state
/// \param text The text to append
///
auto append_content(verse_parse_state& state, const std::string& text) -> void
{
  static constexpr auto add_segment = [](verse_parse_state& state, const std::string& text)
  {
    const auto paragraph_value = state.current_paragraph_id ? parser::html_custom_begin : parser::html_custom_undefined;
    state.segments.push_back(
      {state.current_paragraph_id.value_or(verse_parse_state::paragraph_id_type{}), paragraph_value, text}
    );
  };

  if(!state.verse || !state.chapter)
  {
    LOG_WARN("failed to append content for unknown reference");
    return;
  }
  if(!state.segments.empty())
  {
    const auto same_paragraph = state.current_paragraph_id == state.segments.back().paragraph_id;
    if(!same_paragraph)
    {
      add_segment(state, text);
    }
    else
    {
      state.segments.back().content.append(text);
    }
  }
  else
  {
    add_segment(state, text);
  }
}

///
/// Extract cross-reference texts from a <note style="x"> element's <char style="xt"> children.
/// \param note_node The <note> XML element to extract from
/// \return Vector of non-empty cross-reference text strings
///
auto extract_cross_references(const pugi::xml_node& note_node) -> std::vector<std::string>
{
  auto xrefs = std::vector<std::string>{};
  for(auto note_child : note_node.children())
  {
    if(
      note_child.type() == pugi::node_element && std::string_view(note_child.name()) == "char" &&
      std::string_view(note_child.attribute("style").value()) == "xt"
    )
    {
      auto text = get_inline_text(note_child);
      if(!text.empty())
      {
        xrefs.push_back(std::move(text));
      }
    }
  }
  return xrefs;
}

///
/// Process a single XML element within USX content.
/// Dispatches verse markers, chapter markers, cross-reference notes, and inline styled elements.
/// \param id The book identifier
/// \param element The XML element node to process
/// \param state The current parsing state
/// \param passage_map The map to store completed verses into
///
auto process_element(
  const book_id id, const pugi::xml_node& element, verse_parse_state& state, parser_usx::passage_map_type& passage_map
) -> void
{
  const auto name = std::string_view(element.name());

  if(name == "verse")
  {
    if(const auto attr = element.attribute("number"))
    {
      begin_verse(id, attr.as_uint(), state, passage_map);
    }
  }
  else if(name == "chapter")
  {
    if(const auto attr = element.attribute("number"))
    {
      begin_chapter(id, attr.as_uint(), state, passage_map);
    }
  }
  else if(name == "note" && std::string_view(element.attribute("style").value()) == "x")
  {
    auto xrefs = extract_cross_references(element);
    state.current_xrefs.insert(
      state.current_xrefs.end(), std::make_move_iterator(xrefs.begin()), std::make_move_iterator(xrefs.end())
    );
  }
  else if(name != "note")
  {
    const auto html = serialize_inline_to_html(element);
    if(!html.empty())
    {
      append_content(state, html);
    }
  }
}

///
/// Process a single XML node (text or element) within USX content.
/// Text nodes are appended to the current verse. Element nodes are dispatched via process_element.
/// \param id The book identifier
/// \param node The XML node to process
/// \param state The current parsing state
/// \param passage_map The map to store completed verses into
///
auto process_node(
  const book_id id, const pugi::xml_node& node, verse_parse_state& state, parser_usx::passage_map_type& passage_map
) -> void
{
  const auto type = node.type();
  if(type == pugi::node_pcdata || type == pugi::node_cdata)
  {
    const auto text = std::string(node.value());
    if(!text.empty())
    {
      append_content(state, text);
    }
  }
  else if(type == pugi::node_element)
  {
    const auto name = std::string_view(node.name());
    if(name == "para" && is_content_paragraph(std::string_view(node.attribute("style").value())))
    {
      state.current_paragraph_id = decltype(state.current_paragraph_id)::value_type{};
      for(auto child : node.children())
      {
        process_node(id, child, state, passage_map);
      }
      state.current_paragraph_id = std::nullopt;
    }
    else
    {
      process_element(id, node, state, passage_map);
    }
  }
}

///
/// Parse USX content for a single book into per-verse HTML passages with cross-references.
/// \param id The book identifier
/// \param usx_content The raw USX XML string
/// \return Map of references to html_passage objects, empty on parse failure
///
auto parse_book_passages(const book_id id, const std::string& usx_content) -> parser_usx::passage_map_type
{
  pugi::xml_document doc;
  const auto parse_result = doc.load_string(usx_content.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  if(!parse_result)
  {
    LOG_ERROR("failed to parse USX content for {}: {}", util::enum_name(id), parse_result.description());
    return {};
  }

  const auto usx_node = doc.child("usx");
  if(!usx_node)
  {
    LOG_ERROR("no <usx> root element found for {}", util::enum_name(id));
    return {};
  }

  auto passage_map = parser_usx::passage_map_type{};
  auto state = verse_parse_state{};

  for(auto child : usx_node.children())
  {
    process_node(id, child, state, passage_map);
  }

  flush_verse(id, state, passage_map);
  return passage_map;
}

///
/// Load all passage data for every book from the zip archive.
/// \param zip_reader The zip file reader to load book USX files from
/// \return Map of all references to html_passage objects, empty on failure
///
auto load_book_data(const io::zip_file_reader& zip_reader) -> parser_usx::passage_map_type
{
  SCOPED_TIMER_LOG();
  auto result = parser_usx::passage_map_type{};

  const auto success = std::ranges::all_of(
    parser_usx::books,
    [&](const auto& book)
    {
      const auto& [id, abbreviation] = book;
      const auto content = load_entry(zip_reader, std::format("{}.usx", abbreviation));
      if(!content.has_value() || content->empty())
      {
        LOG_ERROR("failed to load \"{}\" data: expected \"{}.usx\" file within archive", util::enum_name(id), abbreviation);
        return false;
      }
      auto book_result = parse_book_passages(id, *content);
      result.merge(book_result);
      return true;
    }
  );
  if(!success)
  {
    result.clear();
  }
  return result;
}

///
/// Load scripture metadata (name, abbreviation, language, copyright) from the zip archive.
/// \param zip_reader The zip file reader containing metadata.xml
/// \return The loaded scripture information, or std::nullopt on failure
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

} // namespace detail

///
///
parser_usx::parser_usx(const io::zip_file_reader& zip_reader)
  : info_data_{detail::load_info_data(zip_reader)}
{
  verse_data_ = detail::load_book_data(zip_reader);
  if(valid())
  {
    LOG_INFO(
      "loaded scripture: name=\"{}\", abbreviation=\"{}\", language=\"{}\", copyright=\"{}\", verses={}",
      info_data_->name,
      info_data_->abbreviation,
      info_data_->language,
      info_data_->copyright.value_or("not found"),
      verse_data_.size()
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
  return !verse_data_.empty() && info_data_.has_value();
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
  const auto it = verse_data_.find(ref);
  if(it != verse_data_.end())
  {
    return it->second;
  }
  LOG_ERROR("verse not found: {}", ref);
  return std::unexpected(error_code::not_found);
}

} // namespace bibstd::bible
