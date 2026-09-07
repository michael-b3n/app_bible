#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace bibstd::bible::test
{

///
/// Bounding box of a captured OCR element, in the coordinate system of the captured image.
///
struct capture_box final
{
  std::int32_t x{0};
  std::int32_t y{0};
  std::int32_t width{0};
  std::int32_t height{0};

  auto operator==(const capture_box&) const -> bool = default;
};

///
/// Text of a captured OCR element together with its bounding box.
///
struct capture_text final
{
  std::string text;
  capture_box box;

  auto operator==(const capture_text&) const -> bool = default;
};

///
/// Captured word with the line and the paragraph it belongs to, if the engine reported one.
///
struct capture_word final
{
  capture_text word;
  std::optional<capture_text> line;
  std::optional<capture_text> paragraph;
};

///
/// Captured layout analysis element.
///
struct capture_layout final
{
  capture_box line;
  std::optional<capture_box> paragraph;
};

///
/// Everything a real OCR engine reported for one image, enough to replay it without the image.
///
struct capture_data final
{
  std::string id;
  std::uint32_t width{0};
  std::uint32_t height{0};
  std::vector<capture_layout> layouts;
  std::vector<capture_word> words;
};

namespace detail
{

///
/// \return Text with backslashes and line breaks escaped, so that it fits on one line of the capture file
///
inline auto escape(const std::string_view text) -> std::string
{
  auto result = std::string{};
  for(const auto c : text)
  {
    switch(c)
    {
    case '\\': result += "\\\\"; break;
    case '\n': result += "\\n"; break;
    case '\r': result += "\\r"; break;
    default: result += c; break;
    }
  }
  return result;
}

///
/// \return Text with the escaping of \see escape undone
///
inline auto unescape(const std::string_view text) -> std::string
{
  auto result = std::string{};
  for(auto i = std::size_t{0}; i < text.size(); ++i)
  {
    if(text.at(i) != '\\' || i + 1 == text.size())
    {
      result += text.at(i);
      continue;
    }
    ++i;
    switch(text.at(i))
    {
    case 'n': result += '\n'; break;
    case 'r': result += '\r'; break;
    default: result += text.at(i); break;
    }
  }
  return result;
}

///
/// \return Bounding box as the four numbers of one capture file record
///
inline auto to_record(const capture_box& box) -> std::string
{
  return std::to_string(box.x) + " " + std::to_string(box.y) + " " + std::to_string(box.width) + " " +
         std::to_string(box.height);
}

} // namespace detail

///
/// Write the captured data of one image. The format is one record per line. Words refer to the text
/// table by index, so that the paragraph text is not repeated for each of its words.
///
inline auto write_capture(const std::filesystem::path& path, const capture_data& data) -> bool
{
  auto file = std::ofstream{path, std::ios::binary};
  if(!file)
  {
    return false;
  }
  auto texts = std::vector<capture_text>{};
  const auto index_of = [&](const capture_text& text) -> std::string
  {
    const auto it = std::ranges::find(texts, text);
    if(it == std::ranges::cend(texts))
    {
      texts.push_back(text);
      return std::to_string(texts.size() - 1);
    }
    return std::to_string(static_cast<std::size_t>(std::ranges::distance(std::ranges::cbegin(texts), it)));
  };

  auto records = std::ostringstream{};
  for(const auto& word : data.words)
  {
    records << "word " << detail::to_record(word.word.box) << " " << (word.line ? index_of(*word.line) : std::string{"-"})
            << " " << (word.paragraph ? index_of(*word.paragraph) : std::string{"-"}) << " " << detail::escape(word.word.text)
            << "\n";
  }
  for(const auto& layout : data.layouts)
  {
    records << "layout " << detail::to_record(layout.line) << " "
            << (layout.paragraph ? detail::to_record(*layout.paragraph) : std::string{"- - - -"}) << "\n";
  }

  file << "image " << data.width << " " << data.height << "\n";
  for(const auto& text : texts)
  {
    file << "text " << detail::to_record(text.box) << " " << detail::escape(text.text) << "\n";
  }
  file << records.str();
  return true;
}

///
/// \return Captured data read back from a file written by \see write_capture, std::nullopt on a broken file
///
inline auto read_capture(const std::filesystem::path& path) -> std::optional<capture_data>
{
  auto file = std::ifstream{path, std::ios::binary};
  if(!file)
  {
    return std::nullopt;
  }
  auto result = capture_data{.id = path.stem().string()};
  auto texts = std::vector<capture_text>{};
  auto record = std::string{};
  while(std::getline(file, record))
  {
    if(!record.empty() && record.back() == '\r')
    {
      record.pop_back();
    }
    auto stream = std::istringstream{record};
    auto tag = std::string{};
    stream >> tag;

    const auto read_box = [&]
    {
      auto box = capture_box{};
      stream >> box.x >> box.y >> box.width >> box.height;
      return box;
    };
    const auto read_text = [&]
    {
      stream.get(); // separating space
      auto text = std::string{};
      std::getline(stream, text);
      return detail::unescape(text);
    };

    if(tag == "image")
    {
      stream >> result.width >> result.height;
    }
    else if(tag == "text")
    {
      const auto box = read_box();
      texts.emplace_back(capture_text{.text = read_text(), .box = box});
    }
    else if(tag == "word")
    {
      const auto box = read_box();
      auto line_index = std::string{};
      auto paragraph_index = std::string{};
      stream >> line_index >> paragraph_index;
      const auto at = [&](const std::string& index) -> std::optional<capture_text>
      {
        if(index == "-")
        {
          return std::nullopt;
        }
        const auto i = static_cast<std::size_t>(std::stoul(index));
        return i < texts.size() ? std::optional{texts.at(i)} : std::nullopt;
      };
      const auto line = at(line_index);
      const auto paragraph = at(paragraph_index);
      result.words.emplace_back(
        capture_word{
          .word = capture_text{.text = read_text(), .box = box},
            .line = line, .paragraph = paragraph
      }
      );
    }
    else if(tag == "layout")
    {
      const auto box = read_box();
      auto paragraph_x = std::string{};
      stream >> paragraph_x;
      if(paragraph_x == "-")
      {
        result.layouts.emplace_back(capture_layout{.line = box, .paragraph = std::nullopt});
      }
      else
      {
        auto paragraph = capture_box{.x = static_cast<std::int32_t>(std::stol(paragraph_x))};
        stream >> paragraph.y >> paragraph.width >> paragraph.height;
        result.layouts.emplace_back(capture_layout{.line = box, .paragraph = paragraph});
      }
    }
  }
  return result.width > 0 ? std::optional{result} : std::nullopt;
}

///
/// \return Captured data of every capture file in the given folder, empty when there is none
///
inline auto read_captures(const std::filesystem::path& folder) -> std::vector<capture_data>
{
  auto result = std::vector<capture_data>{};
  if(!std::filesystem::is_directory(folder))
  {
    return result;
  }
  for(const auto& entry : std::filesystem::directory_iterator{folder})
  {
    if(entry.path().extension() == std::filesystem::path{".ocr"})
    {
      if(auto data = read_capture(entry.path()))
      {
        result.emplace_back(std::move(*data));
      }
    }
  }
  std::ranges::sort(result, [](const auto& a, const auto& b) { return a.id < b.id; });
  return result;
}

} // namespace bibstd::bible::test
