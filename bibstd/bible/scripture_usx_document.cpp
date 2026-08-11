#include "bibstd/bible/scripture_usx_document.hpp"
#include "bibstd/util/contains.hpp"
#include "bibstd/util/enum.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/string.hpp"
#include "bibstd/util/uid.hpp"

#include <pugixml.hpp>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

namespace bibstd::bible::usx_document
{
namespace
{

///
/// Paragraph styles that introduce a book instead of carrying scripture text. "h" is the running header,
/// "toc1" to "toc3" are the table of contents entries and "mt.." is the major title of the book.
///
// clang-format off
constexpr auto header_paragraph_styles = util::string::to_string_view_array("h", "toc1", "toc2", "toc3", "mt", "mt1", "mt2", "mt3");
// clang-format on

///
/// Header paragraph styles carrying each of the book name forms, most preferred first. USX intends "toc1" as the
/// long form, "toc2" as the short form and "toc3" as the abbreviation. Not every scripture ships all of them, so
/// every form falls back to the closest alternative.
///
// clang-format off
constexpr auto abbreviation_styles = util::string::to_string_view_array("toc3", "h", "toc2");
constexpr auto short_name_styles = util::string::to_string_view_array("toc2", "h", "toc1");
constexpr auto long_name_styles = util::string::to_string_view_array("toc1", "mt1", "mt", "toc2");
// clang-format on

///
/// Check whether a paragraph style introduces the book rather than carrying scripture text.
/// \return true if it is a header paragraph
///
auto is_header_paragraph(const std::string_view style) -> bool
{
  return util::contains(header_paragraph_styles, style);
}

///
/// Remove leading and trailing whitespace.
/// \return trimmed text
///
auto trimmed(const std::string_view text) -> std::string
{
  static constexpr auto is_space = [](const char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };
  const auto begin = std::ranges::find_if_not(text, is_space);
  const auto end = std::ranges::find_if_not(text | std::views::reverse, is_space).base();
  return begin < end ? std::string{begin, end} : std::string{};
}

///
/// Extract text content from an XML node tree.
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
/// Serialize inline USX content to HTML string.
/// Handles char styles (bold, italic, etc.) and skips notes.
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
        if(style == "bd") { result.append(std::format("<b>{}</b>", inner)); }
        else if(style == "it" || style == "em") { result.append(std::format("<i>{}</i>", inner)); }
        else if(style == "nd") { result.append(std::format("<{0}>{1}</{0}>", scripture::html_format_name_of_god, inner)); }
        else if(style == "add") { result.append(std::format("<{0}>{1}</{0}>", scripture::html_format_translator_addition, inner)); }
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
/// Parsing state for tracking the current verse being assembled.
///
struct verse_parse_state final
{
  // Typedefs
  using paragraph_id_type = util::uid<struct paragraph_id_tag>;

  struct segment final
  {
    paragraph_id_type paragraph_id{};
    std::string_view paragraph_attribute_value{scripture::html_custom_attr_value_p_undefined};
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
///
auto flush_verse(const book_id id, verse_parse_state& state, scripture_usx::passage_map_type& passage_map) -> void
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
    html.append(std::format(R"(<p {}="{}">)", scripture::html_custom_attr_name_id, seg.paragraph_attribute_value));
    html.append(seg.content);
    html.append("</p>");
  }
  const auto ref = scripture::reference_type::create_unguarded(id, *state.chapter, *state.verse);
  passage_map.emplace(ref, scripture::passage_html_type{ref, std::move(html), std::move(state.current_xrefs)});
  state.current_xrefs.clear();
  state.segments.clear();
}

///
/// Handle a chapter marker: flush the current verse and begin a new chapter.
///
auto begin_chapter(
  const book_id id, const std::uint32_t chapter_number, verse_parse_state& state, scripture_usx::passage_map_type& passage_map
) -> void
{
  flush_verse(id, state, passage_map);
  state.chapter = chapter_number;
  state.verse = std::nullopt;
}

///
/// Handle a verse marker: flush the current verse and begin a new one.
///
auto begin_verse(
  const book_id id, const std::uint32_t verse_number, verse_parse_state& state, scripture_usx::passage_map_type& passage_map
) -> void
{
  flush_verse(id, state, passage_map);
  state.verse = verse_number;
}

///
/// Append text content to the current verse's active segment.
/// Creates a new segment when the paragraph context changes.
/// Ignores content before any verse starts.
///
auto append_content(verse_parse_state& state, const std::string& text) -> void
{
  if(!state.verse || !state.chapter)
  {
    return;
  }

  static constexpr auto add_segment = [](verse_parse_state& state, const std::string& text)
  {
    const auto paragraph_value =
      state.current_paragraph_id ? scripture::html_custom_attr_value_p_begin : scripture::html_custom_attr_value_p_undefined;
    state.segments.push_back(
      {state.current_paragraph_id.value_or(verse_parse_state::paragraph_id_type{}), paragraph_value, text}
    );
  };

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
/// Dispatches verse markers, chapter markers,
/// cross-reference notes, and inline styled elements.
///
auto process_element(
  const book_id id, const pugi::xml_node& element, verse_parse_state& state, scripture_usx::passage_map_type& passage_map
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
/// Text nodes are appended to the current verse.
/// Element nodes are dispatched via process_element.
///
auto process_node(
  const book_id id, const pugi::xml_node& node, verse_parse_state& state, scripture_usx::passage_map_type& passage_map
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
    if(name == "para" && !is_header_paragraph(std::string_view(node.attribute("style").value())))
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
/// Read the names of the book from the header paragraphs preceding its first chapter.
/// \return Names of the book, forms without a matching header paragraph are empty
///
auto parse_name(const pugi::xml_node& usx_node) -> scripture::book_name_type
{
  auto headers = std::map<std::string, std::string, std::less<>>{};

  const auto is_not_chapter_node = [](const auto& node) { return std::string_view{node.name()} != "chapter"; };
  // the header block introduces the book and therefore ends where its text begins
  for(auto child : usx_node.children() | std::views::take_while(is_not_chapter_node))
  {
    const auto style = std::string_view{child.attribute("style").value()};
    if(std::string_view{child.name()} == "para" && is_header_paragraph(style))
    {
      if(auto text = trimmed(get_inline_text(child)); !text.empty())
      {
        headers.emplace(style, std::move(text)); // a style repeated further down does not override the first occurrence
      }
    }
  }

  const auto preferred = [&headers](const auto& styles)
  {
    for(const auto style : styles)
    {
      if(const auto found = headers.find(style); found != std::cend(headers))
      {
        return found->second;
      }
    }
    return std::string{};
  };
  return scripture::book_name_type{
    .abbreviation = preferred(abbreviation_styles),
    .short_name = preferred(short_name_styles),
    .long_name = preferred(long_name_styles)
  };
}

///
/// Parse the verses of a book into per-verse HTML passages with cross-references.
/// \return Map of references to html_passage objects
///
auto parse_passages(const book_id id, const pugi::xml_node& usx_node) -> scripture_usx::passage_map_type
{
  auto passage_map = scripture_usx::passage_map_type{};
  auto state = verse_parse_state{};

  for(auto child : usx_node.children())
  {
    process_node(id, child, state, passage_map);
  }

  flush_verse(id, state, passage_map);
  return passage_map;
}

} // namespace

///
///
auto parse(const book_id book, const std::string& usx_content) -> std::optional<content>
{
  pugi::xml_document doc;
  const auto parse_result = doc.load_string(usx_content.c_str(), pugi::parse_default | pugi::parse_ws_pcdata);
  if(!parse_result)
  {
    LOG_ERROR("failed to parse USX content for {}: {}", util::enum_name(book), parse_result.description());
    return std::nullopt;
  }

  const auto usx_node = doc.child("usx");
  if(!usx_node)
  {
    LOG_ERROR("no <usx> root element found for {}", util::enum_name(book));
    return std::nullopt;
  }
  return content{.name = parse_name(usx_node), .passages = parse_passages(book, usx_node)};
}

} // namespace bibstd::bible::usx_document
