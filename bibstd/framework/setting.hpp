#pragma once

#include "bibstd/framework/property.hpp"
#include "bibstd/framework/setting_common.hpp"
#include "bibstd/framework/setting_validator.hpp"
#include "bibstd/signal/adapter.hpp"
#include "bibstd/util/visit_helper.hpp"

#include <memory>

namespace bibstd::framework
{

///
/// Struct containing signals for setting class.
///
struct setting_signals final
{
  /// Value changed signal: will be emitted when the setting value changes.
  signal::signal_type<void()> value_changed;
  /// Validator changed signal: will be emitted when the setting validator changes. A change of the setting validator
  /// can change the value of the setting. In the case the signal `value_changed` will be emitted first.
  signal::signal_type<void()> validator_changed;
};

///
/// Setting class.
///
template<underlying_setting_type T>
class setting final : public signal::adapter<setting_signals>
{
  // Variables
  property<T> value_;

public: // Typedefs
  using value_type = T;
  using sptr_type = std::shared_ptr<setting<value_type>>;

public: // Variables
  const std::string path;
  const setting_validator<value_type> validator;

public: // Structors
  setting(const std::string& path, property<value_type>&& value, setting_validator<value_type>&& validator);

public: // Accessors
  ///
  /// Access setting value.
  /// \return reference to setting value
  ///
  auto value() const -> value_type;

public: // Setters
  ///
  /// Set setting value.
  /// \param v setting value that shall be set
  ///
  auto value(const value_type& v) -> bool;

private: // Helpers
  auto validate() -> void;
};

///
///
template<underlying_setting_type T>
setting<T>::setting(const std::string& path_, property<value_type>&& value, setting_validator<value_type>&& validator_)
  : value_{std::move(value)}
  , path{path_}
  , validator{std::move(validator_)}
{
  std::visit(
    [this](const auto& v)
    {
      v->connect_on_changed(
        [this]
        {
          validate();
          notify(&setting_signals::validator_changed);
        }
      );
    },
    validator
  );
  validate();
}

///
///
template<underlying_setting_type T>
auto setting<T>::value() const -> T
{
  return value_.value();
}

///
///
template<underlying_setting_type T>
auto setting<T>::value(const value_type& v) -> bool
{
  return util::visit_lambdas(
    validator,
    [&]([[maybe_unused]] const setting_validator_unbound::sptr_type&) -> bool
    {
      decltype(auto) old_value = value_.exchange(v);
      if(old_value != v)
      {
        notify(&setting_signals::value_changed);
      }
      return true;
    },
    [&](const setting_validator_range<value_type>::sptr_type& validator_range) -> bool
    {
      const auto validated_value = validator_range->validate(v);
      decltype(auto) old_value = value_.exchange(validated_value);
      if(old_value != validated_value)
      {
        notify(&setting_signals::value_changed);
      }
      return validated_value == v;
    },
    [&](const setting_validator_list<value_type>::sptr_type& validator_list) -> bool
    {
      const auto contains = validator_list->contains(v);
      if(contains)
      {
        decltype(auto) old_value = value_.exchange(v);
        if(old_value != v)
        {
          notify(&setting_signals::value_changed);
        }
      }
      return contains;
    }
  );
}

///
///
template<underlying_setting_type T>
auto setting<T>::validate() -> void
{
  util::visit_lambdas(
    validator,
    [&]([[maybe_unused]] const setting_validator_unbound::sptr_type&) { /*noop*/ },
    [&](const setting_validator_range<value_type>::sptr_type& validator_range)
    {
      const auto validated_value = validator_range->validate(value_.value());
      decltype(auto) old_value = value_.exchange(validated_value);
      if(old_value != validated_value)
      {
        notify(&setting_signals::value_changed);
      }
    },
    [&](const setting_validator_list<value_type>::sptr_type& validator_list)
    {
      const auto contains = validator_list->contains(value_.value());
      if(!contains)
      {
        const auto changed = [&]
        {
          if constexpr(setting_validator_list<value_type>::is_vector_type)
          {
            auto new_value = value_.value();
            std::erase_if(new_value, [&](const auto& v) { return !validator_list->contains(v); });
            decltype(auto) old_value = value_.exchange(std::move(new_value));
            return old_value != new_value;
          }
          else if constexpr(setting_validator_list<value_type>::is_optional_type)
          {
            decltype(auto) old_value = value_.exchange(std::nullopt);
            return old_value != std::nullopt;
          }
          else
          {
            const auto new_value = validator_list->available().front();
            decltype(auto) old_value = value_.exchange(new_value);
            return old_value != new_value;
          }
        }();
        if(changed)
        {
          notify(&setting_signals::value_changed);
        }
      }
    }
  );
}

} // namespace bibstd::framework
