#pragma once

#include "meta/contains.hpp"
#include "meta/for_each.hpp"
#include "util/enum.hpp"
#include "util/exception.hpp"

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace bibstd::util
{
namespace detail
{

///
/// Basic settings types.
///
using default_setting_basic_variant = std::variant<
  bool,
  std::int32_t,
  std::int64_t,
  std::uint32_t,
  std::uint64_t,
  double,
  std::string,
  std::chrono::milliseconds,
  std::chrono::seconds,
  std::chrono::minutes,
  std::filesystem::path>;

///
/// Basic settings optional variant.
///
using default_setting_optional_variant = meta::for_each_t<default_setting_basic_variant, std::optional>;

///
/// Basic settings list variant.
///
using default_setting_list_variant = meta::for_each_t<default_setting_basic_variant, std::vector>;

///
/// To enum setting type converter implementation.
///
template<typename E>
  requires(std::is_enum_v<E>)
auto to_type_erased_setting(const E& v) -> std::string
{
  return std::string{to_string_view(v)};
}

///
/// To enum setting type converter implementation.
///
template<typename E>
  requires(std::is_enum_v<E>)
auto to_type_erased_setting(const std::optional<E>& v) -> std::optional<std::string>
{
  return v ? to_type_erased_setting(*v) : std::nullopt;
}

///
/// To enum setting type converter implementation.
///
template<typename E>
  requires(std::is_enum_v<E>)
auto to_type_erased_setting(const std::vector<E>& v) -> auto
{
  auto result = std::vector<std::string>{};
  result.reserve(v.size());
  for(const auto& item : v)
  {
    result.emplace_back(to_type_erased_setting(item));
  }
  return result;
}

///
/// From enum setting type converter implementation.
///
template<typename E>
  requires(std::is_enum_v<E>)
auto from_type_erased_setting(const std::string& v) -> E
{
  const auto value = to_enum<E>(v);
  if(!value)
  {
    THROW_EXCEPTION(exception(std::format("invalid enum setting value: value=\"{}\"", v)));
  }
  return *value;
}

///
/// From enum setting type converter implementation.
///
template<typename T>
  requires(std::is_same_v<T, std::optional<typename T::value_type>> && std::is_enum_v<typename T::value_type>)
auto from_type_erased_setting(const std::optional<std::string>& v) -> T
{
  return v ? from_type_erased_setting<typename T::value_type>(*v) : std::nullopt;
}

///
/// From enum setting type converter implementation.
///
template<typename T>
  requires(std::is_same_v<T, std::vector<typename T::value_type>> && std::is_enum_v<typename T::value_type>)
auto from_type_erased_setting(const std::vector<std::string>& v) -> T
{
  auto result = T{};
  result.reserve(v.size());
  for(const auto& item : v)
  {
    result.emplace_back(from_type_erased_setting<typename T::value_type>(item));
  }
  return result;
}

///
/// Helper concept to detect if a given type has a corresponding erased type.
///
template<typename E>
concept type_erasable = requires(E e) {
  { to_type_erased_setting(e) };
};

} // namespace detail

///
/// All supported setting types as a variant type.
///
using default_setting_variant = meta::combine_pack_t<
  detail::default_setting_basic_variant,
  meta::combine_pack_t<detail::default_setting_optional_variant, detail::default_setting_list_variant>>;

///
/// Concept for all supported base setting types. These types are type erased settings. Supported are:
/// - default types including optional and list variants
///
template<typename T>
concept erased_setting_type = meta::contains_v<default_setting_variant, T>;

///
/// Concept for all supported setting types. Base settings types are a subgroup of this. Supported are:
/// - default types including optional and list variants
/// - enum types
///
template<typename T>
concept underlying_setting_type = erased_setting_type<T> || detail::type_erasable<T>;

///
/// Dummy implementation for from_type_erased_setting for erased setting types.
/// This allows an easy implementation of erased_setting_type_from.
///
namespace detail
{
template<erased_setting_type E>
auto to_type_erased_setting(const E& v) -> E
{
  return v;
}
} // namespace detail

///
/// Type mapping from underlying setting type to type erased setting type.
///
template<underlying_setting_type T>
using erased_setting_type_from =
  std::conditional_t<erased_setting_type<T>, T, decltype(detail::to_type_erased_setting(std::declval<T>()))>;

///
/// Create setting type converter.
/// \param F From type
/// \param T To type
/// \return Converter function from F to T
///
template<underlying_setting_type F, underlying_setting_type T>
  requires(std::is_same_v<erased_setting_type_from<F>, T> || std::is_same_v<F, erased_setting_type_from<T>>)
constexpr auto create_setting_converter() -> auto
{
  if constexpr(std::is_same_v<F, T>)
  {
    return [](const F& v) -> T { return v; };
  }
  else if constexpr(std::is_same_v<erased_setting_type_from<F>, T>)
  {
    return [](const F& v) -> T { return detail::to_type_erased_setting(v); };
  }
  else if constexpr(std::is_same_v<F, erased_setting_type_from<T>>)
  {
    return [](const F& v) -> T { return detail::from_type_erased_setting<T>(v); };
  }
}

} // namespace bibstd::util
