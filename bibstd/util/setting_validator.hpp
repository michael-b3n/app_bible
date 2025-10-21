#pragma once

#include "math/value_range.hpp"
#include "util/setting_common.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <ranges>

namespace bibstd::util
{
namespace detail
{

///
/// Concept for types that can be used as range type in setting_validator_const_range.
///
template<typename T>
concept setting_validator_range_type = underlying_setting_type<T> && std::is_arithmetic_v<T>;

} // namespace detail

///
/// Class for unbound setting validators.
///
struct setting_validator_unbound final
{};

///
/// Class for range type setting validators.
///
template<underlying_setting_type T>
class setting_validator_const_range final
{
public: // Structors
  template<detail::setting_validator_range_type U>
  setting_validator_const_range(U min, U max);

public: // Operators
  ///
  /// Conversion operator to erased setting validator type.
  /// \return erased setting validator type
  ///
  explicit operator setting_validator_const_range<erased_setting_type_from<T>>() const;

public: // Operators
  ///
  /// Validate value and return corrected value if necessary.
  /// \param value Value that shall be validated
  /// \return corrected value if well defined validated value is available, std::nullopt otherwise
  ///
  template<detail::setting_validator_range_type U>
  auto validate(const U& value) const -> U;

public: // Operators
  ///
  /// Check if value is contained within the validator bounds.
  /// \param value Value that shall be checked
  /// \return true if value is contained, false otherwise
  ///
  template<detail::setting_validator_range_type U>
  auto contains(const U& value) const -> bool;

public: // Dummy implementation for unsupported types
  template<typename U>
    requires(!detail::setting_validator_range_type<U>)
  setting_validator_const_range(U min, U max);
  template<typename U>
    requires(!detail::setting_validator_range_type<U>)
  auto validate(const U& value) const -> U;
  template<typename U>
    requires(!detail::setting_validator_range_type<U>)
  auto contains(const U& value) const -> bool;

private: // Variables
  const math::value_range<std::conditional_t<detail::setting_validator_range_type<T>, T, int>> range_;
};

///
/// Class for list type setting validators.
///
template<underlying_setting_type T>
class setting_validator_const_list final
{
public: // Structors
  setting_validator_const_list(std::vector<T> list);
  template<std::ranges::range R>
    requires(std::is_same_v<std::ranges::range_value_t<R>, T>)
  setting_validator_const_list(const R& range);

public: // Operators
  ///
  /// Conversion operator to erased setting validator type.
  /// \return erased setting validator type
  ///
  explicit operator setting_validator_const_list<erased_setting_type_from<T>>() const;

public: // Accessors
  ///
  /// Get all available values.
  /// \return list of all available values
  ///
  auto available() const -> const std::vector<T>&;

public: // Operators
  ///
  /// Check if value is contained within the validator bounds.
  /// \param value Value that shall be checked
  /// \return true if value is contained, false otherwise
  ///
  auto contains(const T& value) const -> bool;

private: // Variables
  const std::vector<T> list_;
};

///
/// Setting validator variant type.
///
template<underlying_setting_type T>
using setting_validator =
  std::variant<setting_validator_unbound, setting_validator_const_range<T>, setting_validator_const_list<T>>;

///
///
template<underlying_setting_type T>
template<detail::setting_validator_range_type U>
setting_validator_const_range<T>::setting_validator_const_range(const U min, const U max)
  : range_{min, max}
{
  if(math::value_range<T>::empty(range_))
  {
    THROW_EXCEPTION(std::invalid_argument("invalid empty range as setting_validator_const_range argument"));
  }
}

///
///
template<underlying_setting_type T>
setting_validator_const_range<T>::operator setting_validator_const_range<erased_setting_type_from<T>>() const
{
  if constexpr(detail::setting_validator_range_type<T>)
  {
    constexpr auto setting_type_converter = create_setting_converter<T, erased_setting_type_from<T>>();
    return setting_validator_const_range<erased_setting_type_from<T>>(
      setting_type_converter(range_.begin), setting_type_converter(range_.end)
    );
  }
  else {
    THROW_EXCEPTION(std::invalid_argument("unsupported range type for conversion to erased setting validator"));
  }
}

///
///
template<underlying_setting_type T>
template<detail::setting_validator_range_type U>
auto setting_validator_const_range<T>::validate(const U& value) const -> U
{
  return math::value_range<T>::clamp(range_, value);
}

///
///
template<underlying_setting_type T>
template<detail::setting_validator_range_type U>
auto setting_validator_const_range<T>::contains(const U& value) const -> bool
{
  return math::value_range<T>::contains(range_, value);
}

///
///
template<underlying_setting_type T>
template<typename U>
  requires(!detail::setting_validator_range_type<U>)
setting_validator_const_range<T>::setting_validator_const_range([[maybe_unused]] U, [[maybe_unused]] U)
{
  THROW_EXCEPTION(std::invalid_argument("unsupported range value type"));
}

///
///
template<underlying_setting_type T>
template<typename U>
  requires(!detail::setting_validator_range_type<U>)
auto setting_validator_const_range<T>::validate(const U& value) const -> U
{
  THROW_EXCEPTION(std::invalid_argument("unsupported value type for validate"));
  return value;
}

///
///
template<underlying_setting_type T>
template<typename U>
  requires(!detail::setting_validator_range_type<U>)
auto setting_validator_const_range<T>::contains([[maybe_unused]] const U&) const -> bool
{
  THROW_EXCEPTION(std::invalid_argument("unsupported value type for contains"));
  return false;
}

///
///
template<underlying_setting_type T>
setting_validator_const_list<T>::setting_validator_const_list(std::vector<T> list)
  : list_{[&]
          {
            std::sort(list.begin(), list.end());
            list.erase(std::unique(list.begin(), list.end()), list.end());
            return list;
          }()}
{
  if(list_.empty())
  {
    THROW_EXCEPTION(std::invalid_argument("invalid empty list as setting_validator_const_list argument"));
  }
}

///
///
template<underlying_setting_type T>
setting_validator_const_list<T>::operator setting_validator_const_list<erased_setting_type_from<T>>() const
{
  constexpr auto setting_type_converter = create_setting_converter<T, erased_setting_type_from<T>>();
  auto converted_list = std::vector<erased_setting_type_from<T>>{};
  converted_list.reserve(list_.size());
  for(const auto& item : list_)
  {
    converted_list.emplace_back(setting_type_converter(item));
  }
  return setting_validator_const_list<erased_setting_type_from<T>>{std::move(converted_list)};
}

///
///
template<underlying_setting_type T>
template<std::ranges::range R>
  requires(std::is_same_v<std::ranges::range_value_t<R>, T>)
setting_validator_const_list<T>::setting_validator_const_list(const R& range)
  : setting_validator_const_list<T>({std::ranges::begin(range), std::ranges::end(range)})
{
}

///
///
template<underlying_setting_type T>
auto setting_validator_const_list<T>::available() const -> const std::vector<T>&
{
  return list_;
}

///
///
template<underlying_setting_type T>
auto setting_validator_const_list<T>::contains(const T& value) const -> bool
{
  return std::ranges::binary_search(list_, value);
}

} // namespace bibstd::util
