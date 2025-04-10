#pragma once

#include "meta/chrono.hpp"
#include "meta/contains.hpp"
#include "meta/pack.hpp"
#include "util/enum.hpp"

#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cstdint>
#include <format>
#include <ranges>
#include <string>

namespace bibstd::util
{

///
/// All valid property_core types in a pack.
///
using basic_property_value_types = meta::pack<
  bool,
  std::int8_t,
  std::uint8_t,
  std::int16_t,
  std::uint16_t,
  std::int32_t,
  std::uint32_t,
  std::int64_t,
  std::uint64_t,
  float,
  double,
  std::string>;

///
/// Property type concept.
///
template<typename T>
concept basic_property_value_type = meta::contains_v<basic_property_value_types, std::remove_cv_t<T>>;

///
/// Basic property tree type.
///
using property_tree_type = boost::property_tree::ptree;
using property_path_type = property_tree_type::path_type;

///
/// Pair type concept.
///
template<typename T>
concept property_parser_pair_type = requires(T) {
  T::first;
  T::second;
};

///
/// Property parser defines read and write operations from user defined type to basic property value type.
/// The reader returns the value stored in tree if successfully read, else it must return the default value.
/// The writer writes the provided value to the tree.
///
struct property_parser final
{
  template<basic_property_value_type T>
  static auto read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>;
  template<basic_property_value_type T>
  static auto write(const property_path_type& path, property_tree_type& tree, const T& value) -> void;

  template<enum_type T>
  static auto read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>;
  template<enum_type T>
  static auto write(const property_path_type& path, property_tree_type& tree, const T& value) -> void;

  template<property_parser_pair_type T>
  static auto read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>;
  template<property_parser_pair_type T>
  static auto write(const property_path_type& path, property_tree_type& tree, const T& value) -> void;

  template<typename T>
    requires meta::is_duration_v<T>
  static auto read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>;
  template<typename T>
    requires meta::is_duration_v<T>
  static auto write(const property_path_type& path, property_tree_type& tree, const T& value) -> void;

  template<typename Container>
  static auto read(const property_path_type& path, const property_tree_type& tree) -> std::optional<Container>;
  template<typename Container>
  static auto write(const property_path_type& path, property_tree_type& tree, const Container& value) -> void;
};

///
///
template<basic_property_value_type T>
auto property_parser::read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>
{
  const auto v = tree.get_optional<T>(path);
  return v.has_value() ? v.value() : std::optional<T>{};
}

///
///
template<basic_property_value_type T>
auto property_parser::write(const property_path_type& path, property_tree_type& tree, const T& value) -> void
{
  tree.put(path, value);
}

///
///
template<enum_type T>
auto property_parser::read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>
{
  const auto optional_value = tree.get_optional<std::string>(path);
  if(optional_value.has_value())
  {
    if(const auto v = to_enum<T>(optional_value.value()); v.has_value())
    {
      return v.value();
    }
  }
  return std::nullopt;
}

///
///
template<enum_type T>
auto property_parser::write(const property_path_type& path, property_tree_type& tree, const T& value) -> void
{
  tree.put(path, to_string_view(value));
}

///
///
template<property_parser_pair_type T>
auto property_parser::read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>
{
  const auto first = read<T::first_type>(path / "first", tree);
  const auto second = read<T::second_type>(path / "second", tree);
  return first.has_value() && second.has_value() ? T{first.value(), second.value()} : std::optional<T>{};
}

///
///
template<property_parser_pair_type T>
auto property_parser::write(const property_path_type& path, property_tree_type& tree, const T& value) -> void
{
  write(path / "first", tree, value.first);
  write(path / "second", tree, value.second);
}

///
///
template<typename T>
  requires meta::is_duration_v<T>
auto property_parser::read(const property_path_type& path, const property_tree_type& tree) -> std::optional<T>
{
  const auto duration = read<std::int64_t>(path, tree);
  return duration.has_value() ? std::chrono::duration_cast<T>(std::chrono::nanoseconds{duration.value()}) : std::optional<T>{};
}

///
///
template<typename T>
  requires meta::is_duration_v<T>
auto property_parser::write(const property_path_type& path, property_tree_type& tree, const T& value) -> void
{
  write(path, tree, std::chrono::duration_cast<std::chrono::nanoseconds>(value).count());
}

///
///
template<typename Container>
auto property_parser::read(const property_path_type& path, const property_tree_type& tree) -> std::optional<Container>
{
  const auto size = read<std::uint64_t>(path / "size", tree);
  if(!size.has_value())
  {
    return std::nullopt;
  }
  auto retval = Container(size.value());
  const auto valid = std::ranges::all_of(
    std::views::iota(decltype(size.value()){0}, size.value()),
    [&](const auto i)
    {
      const auto element = read<typename Container::value_type>(path / std::format("index_{}", i), tree);
      const auto success = element.has_value();
      if(success)
      {
        retval.at(i) = element.value();
      }
      return success;
    }
  );
  return valid ? retval : std::optional<Container>{};
}

///
///
template<typename Container>
auto property_parser::write(const property_path_type& path, property_tree_type& tree, const Container& value) -> void
{
  write(path / "size", tree, static_cast<std::uint64_t>(value.size()));
  std::ranges::for_each(
    std::views::iota(decltype(value.size()){0}, value.size()),
    [&](const auto i) { write(path / std::format("index_{}", i), tree, value.at(i)); }
  );
}

} // namespace bibstd::util
