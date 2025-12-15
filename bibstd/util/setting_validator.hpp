#pragma once

#include "bibstd/math/value_range.hpp"
#include "bibstd/util/setting_common.hpp"

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <ranges>

namespace bibstd::util
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

private: // Variables
  mutable std::mutex mtx_{};
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
  static constexpr auto to_type_erased_value = create_setting_value_converter<T, setting_type_erased_type_from<T>>();
  static constexpr auto from_type_erased_value = create_setting_value_converter<setting_type_erased_type_from<T>, T>();

public: // Structors
  setting_validator_range(T min, T max)
    requires(detail::setting_validator_range_type<T>);

public: // Operators
  ///
  /// Validate value and return corrected value if necessary.
  /// \param value Value that shall be validated
  /// \return corrected value if well defined validated value is available, std::nullopt otherwise
  ///
  auto validate(const T& value) const -> T
    requires(detail::setting_validator_range_type<T>);

public: // Operators
  ///
  /// Check if value is contained within the validator bounds.
  /// \param value Value that shall be checked
  /// \return true if value is contained, false otherwise
  ///
  auto contains(const T& value) const -> bool
    requires(detail::setting_validator_range_type<T>);

public: // Dummy implementation for unsupported types
  setting_validator_range(T min, T max)
    requires(!detail::setting_validator_range_type<T>);
  auto validate(const T& value) const -> T
    requires(!detail::setting_validator_range_type<T>);
  auto contains(const T& value) const -> bool
    requires(!detail::setting_validator_range_type<T>);

private: // Variables
  const math::value_range<std::conditional_t<detail::setting_validator_range_type<T>, T, int>> range_;
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
  auto validate(const T& value) const -> T;

public: // Operators
  ///
  /// \see setting_validator_range::contains
  ///
  auto contains(const T& value) const -> bool;

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
  static constexpr auto to_type_erased_value = create_setting_value_converter<T, setting_type_erased_type_from<T>>();
  static constexpr auto from_type_erased_value = create_setting_value_converter<setting_type_erased_type_from<T>, T>();

public: // Structors
  setting_validator_list(std::vector<T> list);
  template<std::ranges::range R>
    requires(std::is_same_v<std::ranges::range_value_t<R>, T>)
  setting_validator_list(const R& range);

public: // Accessors
  ///
  /// Get all available values.
  /// \return list of all available values
  ///
  auto available() const -> std::vector<T>;

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
  auto available() const -> std::vector<T>;

public: // Operators
  ///
  /// \see setting_validator_list::contains
  ///
  auto contains(const T& value) const -> bool;

public: // Variables
  const std::shared_ptr<detail::setting_validator_connector> connector;

private: // Variables
  const std::function<std::vector<T>()> available_;
  const std::function<bool(const T&)> contains_;
};

///
/// Setting validator variant type.
///
template<underlying_setting_type T>
using setting_validator = std::variant<
  setting_validator_unbound::sptr_type,
  typename setting_validator_range<T>::sptr_type,
  typename setting_validator_list<T>::sptr_type>;

///
/// Setting validator type erased variant type.
///
template<underlying_setting_type T>
using setting_validator_type_erased = std::variant<
  setting_validator_unbound::sptr_type,
  typename setting_validator_range_type_erased<T>::sptr_type,
  typename setting_validator_list_type_erased<T>::sptr_type>;

///
///
template<underlying_setting_type T>
setting_validator_range<T>::setting_validator_range(const T min, const T max)
  requires(detail::setting_validator_range_type<T>)
  : range_{min, max}
{
  if(math::value_range<T>::empty(range_))
  {
    THROW_EXCEPTION("invalid empty range as setting_validator_range argument");
  }
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::validate(const T& value) const -> T
  requires(detail::setting_validator_range_type<T>)
{
  return math::value_range<T>::clamp(range_, value);
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::contains(const T& value) const -> bool
  requires(detail::setting_validator_range_type<T>)
{
  return math::value_range<T>::contains(range_, value);
}

///
///
template<underlying_setting_type T>
setting_validator_range<T>::setting_validator_range([[maybe_unused]] T, [[maybe_unused]] T)
  requires(!detail::setting_validator_range_type<T>)
{
  THROW_EXCEPTION("unsupported range value type");
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::validate(const T& value) const -> T
  requires(!detail::setting_validator_range_type<T>)
{
  THROW_EXCEPTION("unsupported value type for validate");
  return value;
}

///
///
template<underlying_setting_type T>
auto setting_validator_range<T>::contains([[maybe_unused]] const T&) const -> bool
  requires(!detail::setting_validator_range_type<T>)
{
  THROW_EXCEPTION("unsupported value type for contains");
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
setting_validator_list<T>::setting_validator_list(std::vector<T> list)
  : list_{[&]
          {
            std::sort(list.begin(), list.end());
            list.erase(std::unique(list.begin(), list.end()), list.end());
            return list;
          }()}
{
  if(list_.empty())
  {
    THROW_EXCEPTION("invalid empty list as setting_validator_list argument");
  }
}

///
///
template<underlying_setting_type T>
template<std::ranges::range R>
  requires(std::is_same_v<std::ranges::range_value_t<R>, T>)
setting_validator_list<T>::setting_validator_list(const R& range)
  : setting_validator_list<T>({std::ranges::begin(range), std::ranges::end(range)})
{
}

///
///
template<underlying_setting_type T>
auto setting_validator_list<T>::available() const -> std::vector<T>
{
  return list_;
}

///
///
template<underlying_setting_type T>
auto setting_validator_list<T>::contains(const T& value) const -> bool
{
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
                 auto converted_list = std::vector<T>{};
                 converted_list.reserve(available_list.size());
                 for(const auto& item : available_list)
                 {
                   converted_list.emplace_back(detail::validator_t<decltype(validator)>::to_type_erased_value(item));
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
auto setting_validator_list_type_erased<T>::available() const -> std::vector<T>
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

} // namespace bibstd::util
