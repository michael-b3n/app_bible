#pragma once

#include "bibstd/framework/setting_common.hpp"
#include "bibstd/math/value_range.hpp"
#include "bibstd/meta/type_traits.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <ranges>

namespace bibstd::framework
{
namespace detail
{

///
/// Concept for types that can be used as range type in setting_validator_range.
///
template<typename T>
concept setting_validator_range_type = underlying_setting_type<T> && std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

///
/// Helper type to extract validator type from smart pointer.
///
template<typename T>
using validator_t = std::decay_t<T>::element_type;

///
/// Helper concept to check if type erased validator can be constructed from specific validator type.
///
template<typename T, template<typename...> typename VT, typename V>
concept type_erased_validator_param_requirement =
  std::is_same_v<detail::validator_t<V>, VT<typename validator_t<V>::underlying_type>> &&
  std::is_same_v<T, setting_type_erased_type_from<typename validator_t<V>::underlying_type>>;

///
/// Connector class for setting validators. This class shall be used as base class for all setting validators.
///
class setting_validator_connector
{
public: // Structors
  setting_validator_connector() = default;
  virtual ~setting_validator_connector() = default;

public: // Accessors
  ///
  /// Register a callback to be called when the setting validator changes.
  /// \param on_changed Callback that shall be called when the setting validator changes
  ///
  auto connect_on_changed(std::function<void()> on_changed) -> void;

protected: // Functions
  ///
  /// Notify that the setting validator has changed.
  ///
  auto notify_changed() -> void;

protected: // Variables
  mutable std::mutex mtx_{};

private: // Variables
  std::function<void()> on_changed_{nullptr};
};

} // namespace detail

///
/// Class for unbound setting validators.
///
struct setting_validator_unbound final : public detail::setting_validator_connector
{
  // Typedefs
  using sptr_type = std::shared_ptr<setting_validator_unbound>;
};

///
/// Class for range type setting validators.
///
template<underlying_setting_type T>
class setting_validator_range final : public detail::setting_validator_connector
{
public: // Typedefs
  using sptr_type = std::shared_ptr<setting_validator_range<T>>;
  using underlying_type = T;

public: // Constants
  static constexpr auto is_optional_setting_type = meta::is_optional_v<T>;
  static constexpr auto to_type_erased_value = create_setting_value_converter<T, setting_type_erased_type_from<T>>();
  static constexpr auto from_type_erased_value = create_setting_value_converter<setting_type_erased_type_from<T>, T>();

public: // Structors
  setting_validator_range(meta::remove_optional_t<T> min, meta::remove_optional_t<T> max)
    requires(detail::setting_validator_range_type<meta::remove_optional_t<T>>);

public: // Operators
  ///
  /// Validate value and return corrected value if necessary.
  /// \param value Value that shall be validated
  /// \return corrected value if well defined validated value is available, std::nullopt otherwise
  ///
  [[nodiscard]] auto validate(const T& value) const -> T
    requires(detail::setting_validator_range_type<T>);

public: // Operators
  ///
  /// Check if value is contained within the validator bounds.
  /// \param value Value that shall be checked
  /// \return true if value is contained, false otherwise
  ///
  [[nodiscard]] auto contains(const T& value) const -> bool
    requires(detail::setting_validator_range_type<T>);

public: // Dummy implementation for unsupported types
  setting_validator_range(meta::remove_optional_t<T> min, meta::remove_optional_t<T> max)
    requires(!detail::setting_validator_range_type<T>);
  auto validate(const T& value) const -> T
    requires(!detail::setting_validator_range_type<T>);
  auto contains(const T& value) const -> bool
    requires(!detail::setting_validator_range_type<T>);

private: // Variables
  const math::value_range<
    std::conditional_t<detail::setting_validator_range_type<meta::remove_optional_t<T>>, meta::remove_optional_t<T>, int>>
    range_;
};

///
/// Class for range type setting validators for type erased types.
///
template<underlying_setting_type_erased_type T>
class setting_validator_range_type_erased final
{
public: // Typedefs
  using sptr_type = std::shared_ptr<setting_validator_range_type_erased<T>>;
  using underlying_type = T;

public: // Structors
  setting_validator_range_type_erased(const auto& validator)
    requires(detail::type_erased_validator_param_requirement<T, setting_validator_range, decltype(validator)>);

public: // Operators
  ///
  /// \see setting_validator_range::validate
  ///
  [[nodiscard]] auto validate(const T& value) const -> T;

public: // Operators
  ///
  /// \see setting_validator_range::contains
  ///
  [[nodiscard]] auto contains(const T& value) const -> bool;

public: // Variables
  const std::shared_ptr<detail::setting_validator_connector> connector;

private: // Variables
  const std::function<T(const T&)> validate_;
  const std::function<bool(const T&)> contains_;
};

///
/// Class for list type setting validators.
///
template<underlying_setting_type T>
class setting_validator_list final : public detail::setting_validator_connector
{
public: // Typedefs
  using sptr_type = std::shared_ptr<setting_validator_list<T>>;
  using underlying_type = T;

public: // Constants
  static constexpr auto is_optional_setting_type = meta::is_optional_v<T>;
  static constexpr auto to_type_erased_value = create_setting_value_converter<T, setting_type_erased_type_from<T>>();
  static constexpr auto from_type_erased_value = create_setting_value_converter<setting_type_erased_type_from<T>, T>();

public: // Structors
  setting_validator_list() = default;
  setting_validator_list(std::vector<meta::remove_optional_t<T>> list);
  template<std::ranges::range R>
  setting_validator_list(const R& range)
    requires(std::is_same_v<std::ranges::range_value_t<R>, meta::remove_optional_t<T>>);

public: // Accessors
  ///
  /// Get all available values.
  /// \return list of all available values
  ///
  [[nodiscard]] auto available() const -> std::vector<meta::remove_optional_t<T>>;

public: // Modifiers
  ///
  /// Set the available values. This will replace all previously available values.
  /// \param list List of available values
  /// \return true if list is valid and has been set, false otherwise
  ///
  template<std::ranges::range R>
  [[nodiscard]] auto available(const R& range) -> bool
    requires(std::is_same_v<std::ranges::range_value_t<R>, meta::remove_optional_t<T>>);

public: // Operators
  ///
  /// Check if value is contained within the validator bounds.
  /// \param value Value that shall be checked
  /// \return true if value is contained, false otherwise
  ///
  [[nodiscard]] auto contains(const T& value) const -> bool;

private: // Variables
  std::vector<meta::remove_optional_t<T>> list_{};
};

///
/// Class for list type setting validators for type erased types.
///
template<underlying_setting_type_erased_type T>
class setting_validator_list_type_erased final
{
public: // Typedefs
  using sptr_type = std::shared_ptr<setting_validator_list_type_erased<T>>;
  using underlying_type = T;

public: // Structors
  setting_validator_list_type_erased(const auto& validator)
    requires(detail::type_erased_validator_param_requirement<T, setting_validator_list, decltype(validator)>);

public: // Accessors
  ///
  /// \see setting_validator_list::available
  ///
  [[nodiscard]] auto available() const -> std::vector<meta::remove_optional_t<T>>;

public: // Operators
  ///
  /// \see setting_validator_list::contains
  ///
  [[nodiscard]] auto contains(const T& value) const -> bool;

public: // Variables
  const std::shared_ptr<detail::setting_validator_connector> connector;

private: // Variables
  const std::function<std::vector<meta::remove_optional_t<T>>()> available_;
  const std::function<bool(const T&)> contains_;
};

///
/// Setting validator variant type.
/// Optional underlying setting types are stripped away for validators.
///
template<underlying_setting_type T>
using setting_validator = std::variant<
  setting_validator_unbound::sptr_type,
  typename setting_validator_range<T>::sptr_type,
  typename setting_validator_list<T>::sptr_type>;

///
/// Setting validator type erased variant type.
/// Optional underlying setting types are stripped away for validators.
///
template<underlying_setting_type T>
using setting_validator_type_erased = std::variant<
  setting_validator_unbound::sptr_type,
  typename setting_validator_range_type_erased<T>::sptr_type,
  typename setting_validator_list_type_erased<T>::sptr_type>;

///
///
template<underlying_setting_type T>
setting_validator_range<T>::setting_validator_range(const meta::remove_optional_t<T> min, const meta::remove_optional_t<T> max)
  requires(detail::setting_validator_range_type<meta::remove_optional_t<T>>)
  : range_{min, max}
{
  if constexpr(!is_optional_setting_type)
  {
    if(math::value_range<meta::remove_optional_t<T>>::empty(range_))
    {
      throw util::exception{"invalid empty range argument"};
    }
  }
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::validate(const T& value) const -> T
  requires(detail::setting_validator_range_type<T>)
{
  if constexpr(is_optional_setting_type)
  {
    if(math::value_range<meta::remove_optional_t<T>>::empty(range_))
    {
      return std::nullopt;
    }
  }
  // range must not be empty for non-optional setting types
  // \see setting_validator_range::setting_validator_range
  return math::value_range<meta::remove_optional_t<T>>::clamp(range_, value);
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::contains(const T& value) const -> bool
  requires(detail::setting_validator_range_type<T>)
{
  if constexpr(is_optional_setting_type)
  {
    if(math::value_range<meta::remove_optional_t<T>>::empty(range_))
    {
      return true;
    }
  }
  return math::value_range<T>::contains(range_, value);
}

///
///
template<underlying_setting_type T>
setting_validator_range<T>::setting_validator_range(
  [[maybe_unused]] meta::remove_optional_t<T>, [[maybe_unused]] meta::remove_optional_t<T>
)
  requires(!detail::setting_validator_range_type<T>)
{
  throw util::exception("unsupported range value type");
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::validate(const T& value) const -> T
  requires(!detail::setting_validator_range_type<T>)
{
  throw util::exception("unsupported range value type");
  return value;
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::contains([[maybe_unused]] const T&) const -> bool
  requires(!detail::setting_validator_range_type<T>)
{
  throw util::exception("unsupported range value type");
  return false;
}

///
///
template<underlying_setting_type_erased_type T>
setting_validator_range_type_erased<T>::setting_validator_range_type_erased(const auto& validator)
  requires(detail::type_erased_validator_param_requirement<T, setting_validator_range, decltype(validator)>)
  : connector{validator}
  , validate_{[validator](const T& value)
              {
                return detail::validator_t<decltype(validator)>::to_type_erased_value(
                  validator->validate(detail::validator_t<decltype(validator)>::from_type_erased_value(value))
                );
              }}
  , contains_{[validator](const T& value)
              { return validator->contains(detail::validator_t<decltype(validator)>::from_type_erased_value(value)); }}
{
}

///
///
template<underlying_setting_type_erased_type T>
auto setting_validator_range_type_erased<T>::validate(const T& value) const -> T
{
  return validate_(value);
}

///
///
template<underlying_setting_type_erased_type T>
auto setting_validator_range_type_erased<T>::contains(const T& value) const -> bool
{
  return contains_(value);
}

///
///
template<underlying_setting_type T>
setting_validator_list<T>::setting_validator_list(std::vector<meta::remove_optional_t<T>> list)
  : list_{[&]
          {
            std::sort(list.begin(), list.end());
            list.erase(std::unique(list.begin(), list.end()), list.end());
            return list;
          }()}
{
  if constexpr(!is_optional_setting_type)
  {
    if(list_.empty())
    {
      throw util::exception{"invalid empty list argument"};
    }
  }
}

///
///
template<underlying_setting_type T>
template<std::ranges::range R>
setting_validator_list<T>::setting_validator_list(const R& range)
  requires(std::is_same_v<std::ranges::range_value_t<R>, meta::remove_optional_t<T>>)
  : setting_validator_list<T>({std::ranges::begin(range), std::ranges::end(range)})
{
}

///
///
template<underlying_setting_type T>
auto setting_validator_list<T>::available() const -> std::vector<meta::remove_optional_t<T>>
{
  const auto lock = std::scoped_lock{mtx_};
  return list_;
}

///
///
template<underlying_setting_type T>
template<std::ranges::range R>
auto setting_validator_list<T>::available(const R& range) -> bool
  requires(std::is_same_v<std::ranges::range_value_t<R>, meta::remove_optional_t<T>>)
{
  auto new_list = [&range]
  {
    auto list = std::vector<meta::remove_optional_t<T>>{std::ranges::begin(range), std::ranges::end(range)};
    std::sort(list.begin(), list.end());
    list.erase(std::unique(list.begin(), list.end()), list.end());
    return list;
  }();
  const auto new_list_valid = is_optional_setting_type || !new_list.empty();
  if constexpr(is_optional_setting_type)
  {
    const auto lock = std::scoped_lock{mtx_};
    list_ = std::move(new_list);
  }
  else
  {
    if(new_list_valid)
    {
      const auto lock = std::scoped_lock{mtx_};
      list_ = std::move(new_list);
    }
  }
  notify_changed();
  return new_list_valid;
}

///
///
template<underlying_setting_type T>
auto setting_validator_list<T>::contains(const T& value) const -> bool
{
  const auto lock = std::scoped_lock{mtx_};
  return std::ranges::binary_search(list_, value);
}

///
///
template<underlying_setting_type_erased_type T>
setting_validator_list_type_erased<T>::setting_validator_list_type_erased(const auto& validator)
  requires(detail::type_erased_validator_param_requirement<T, setting_validator_list, decltype(validator)>)
  : connector{validator}
  , available_{[validator]()
               {
                 const auto available_list = validator->available();
                 auto converted_list = std::vector<meta::remove_optional_t<T>>{};
                 converted_list.reserve(available_list.size());
                 for(const auto& item : available_list)
                 {
                   auto converted_item = detail::validator_t<decltype(validator)>::to_type_erased_value(item);
                   if constexpr(detail::validator_t<decltype(validator)>::is_optional_setting_type)
                   {
                     // guaranteed to be valid since the original list does not contain std::nullopt values
                     converted_list.emplace_back(std::move(converted_item.value()));
                   }
                   else
                   {
                     converted_list.emplace_back(std::move(converted_item));
                   }
                 }
                 return converted_list;
               }}
  , contains_{[validator](const T& value)
              { return validator->contains(detail::validator_t<decltype(validator)>::from_type_erased_value(value)); }}
{
}

///
///
template<underlying_setting_type_erased_type T>
auto setting_validator_list_type_erased<T>::available() const -> std::vector<meta::remove_optional_t<T>>
{
  return available_();
}

///
///
template<underlying_setting_type_erased_type T>
auto setting_validator_list_type_erased<T>::contains(const T& value) const -> bool
{
  return contains_(value);
}

} // namespace bibstd::framework
