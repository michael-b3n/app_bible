#include "txt/indexed_strings.hpp"
#include "util/log.hpp"
#include "util/string.hpp"

#include <algorithm>
#include <ranges>

namespace bibstd::txt
{

///
///
auto indexed_strings::size() const -> std::size_t
{
  return data_.size();
}

///
///
auto indexed_strings::read_at(const std::size_t index) const -> std::string
{
  return data_.at(index).second;
}

///
///
auto indexed_strings::joined_string() const -> std::string
{
  auto result = std::string{""};
  std::ranges::for_each(data_, [&](const auto& p) { result.append(p.second); });
  return result;
}

///
///
auto indexed_strings::indexed_chars() const -> std::vector<std::pair<std::size_t, char>>
{
  auto result = std::vector<std::pair<std::size_t, char>>{};
  result.reserve(data_.size());
  std::ranges::for_each(
    data_,
    [&](const auto& p)
    { std::ranges::for_each(p.second, [&](const auto character) { result.emplace_back(p.first, character); }); }
  );
  return result;
}

///
///
auto indexed_strings::append_string(const std::string_view character) -> void
{
  data_.emplace_back(data_.size(), util::to_string(character));
}

///
///
auto indexed_strings::overwrite_at(const std::size_t index, const std::string_view character) -> void
{
  data_.at(index).second = util::to_string(character);
}

} // namespace bibstd::txt
