#pragma once

#include "meta/contains.hpp"
#include "meta/pack.hpp"
#include "meta/type_traits.hpp"

#include <boost/property_tree/ptree.hpp>
#include <magic_enum/magic_enum.hpp>

#include <cstdint>
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
using basic_property_tree_type = boost::property_tree::ptree;
using basic_property_path_type = basic_property_tree_type::path_type;

///
/// Property parser defines read and write operations from user defined type to basic property value type.
/// The reader returns the value stored in tree if successfully read, else it must return the default value.
/// The writer writes the provided value to the tree.
///
struct property_parser final
{
  template<basic_property_value_type T>
  static auto read(const basic_property_path_type& path, const basic_property_tree_type& tree, T&& default_value) -> T;
  template<basic_property_value_type T>
  static auto write(const basic_property_path_type& path, basic_property_tree_type& tree, const T& value) -> void;

  template<meta::enum_type T>
  static auto read(const basic_property_path_type& path, const basic_property_tree_type& tree, T&& default_value) -> T;
  template<meta::enum_type T>
  static auto write(const basic_property_path_type& path, basic_property_tree_type& tree, const T& value) -> void;

  template<meta::pair_type T>
  static auto read(const basic_property_path_type& path, const basic_property_tree_type& tree, T&& default_value) -> T;
  template<meta::pair_type T>
  static auto write(const basic_property_path_type& path, basic_property_tree_type& tree, const T& value) -> void;
};

///
///
template<basic_property_value_type T>
auto property_parser::read(const basic_property_path_type& path, const basic_property_tree_type& tree, T&& default_value) -> T
{
  const auto v = tree.get_optional<T>(path);
  return v.value_or(default_value);
}

///
///
template<basic_property_value_type T>
auto property_parser::write(const basic_property_path_type& path, basic_property_tree_type& tree, const T& value) -> void
{
  tree.put(path, value);
}

///
///
template<meta::enum_type T>
auto property_parser::read(const basic_property_path_type& path, const basic_property_tree_type& tree, T&& default_value) -> T
{
  const auto optional_value = tree.get_optional<std::string>(path);
  if(optional_value.has_value())
  {
    if(const auto v = magic_enum::enum_cast<T>(optional_value.value()); v.has_value())
    {
      return v.value();
    }
  }
  return default_value;
}

///
///
template<meta::enum_type T>
auto property_parser::write(const basic_property_path_type& path, basic_property_tree_type& tree, const T& value) -> void
{
  tree.put(path, magic_enum::enum_name(value));
}

///
///
template<meta::pair_type T>
auto property_parser::read(const basic_property_path_type& path, const basic_property_tree_type& tree, T&& default_value) -> T
{
  return T{read(path / "first", tree, std::move(default_value.first)), read(path / "second", tree, std::move(default_value.second))};
}

///
///
template<meta::pair_type T>
auto property_parser::write(const basic_property_path_type& path, basic_property_tree_type& tree, const T& value) -> void
{
  write(path / "first", tree, value.first);
  write(path / "second", tree, value.second);
}

} // namespace bibstd::util
