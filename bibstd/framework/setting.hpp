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
/// Signaling IDs for setting class.
///
enum class setting_signal_id
{
  /// Value changed signal: will be emitted when the setting value changes.
  value_changed,
  /// Validator changed signal: will be emitted when the setting validator changes. A change of the setting validator
  /// can change the value of the setting. In the case the signal `value_changed` will be emitted first.
  validator_changed,
};

///
/// Setting signal adapter type.
///
using setting_signal_adapter = signal::adapter<
  signal::named_signal<setting_signal_id::value_changed, signal::signal_type<void()>>,
  signal::named_signal<setting_signal_id::validator_changed, signal::signal_type<void()>>>;

///
/// Setting class.
///
template<underlying_setting_type T>
class setting final : public setting_signal_adapter
{
public: // Typedefs
  using value_type = T;
  using sptr_type = std::shared_ptr<setting<T>>;

public: // Structors
  setting(const std::string& path, property<T>&& value, setting_validator<T>&& validator);

public: // Accessors
  ///
  /// Access setting value.
  /// \return reference to setting value
  ///
  auto value() const -> T;

public: // Setters
  ///
  /// Set setting value.
  /// \param v setting value that shall be set
  ///
  auto value(const T& v) -> bool;

public: // Variables
  const std::string path;
  const setting_validator<T> validator;

private: // Variables
  property<T> value_;
};

///
///
template<underlying_setting_type T>
setting<T>::setting(const std::string& path_, property<T>&& value, setting_validator<T>&& validator_)
  : path{path_}
  , validator{std::move(validator_)}
  , value_{std::move(value)}
{
  std::visit(
    [this](const auto& v)
    {
      v->connect_on_changed(
        [this]
        {
          this->value(value_.value());
          emit<setting_signal_id::validator_changed>();
        }
      );
    },
    validator
  );
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
auto setting<T>::value(const T& v) -> bool
{
  return util::visit_lambdas(
    validator,
    [&]([[maybe_unused]] const setting_validator_unbound::sptr_type&) -> bool
    {
      decltype(auto) old_value = value_.exchange(v);
      if(old_value != v)
      {
        emit<setting_signal_id::value_changed>();
      }
      return true;
    },
    [&](const setting_validator_range<T>::sptr_type& validator_range) -> bool
    {
      const auto validated_value = validator_range->validate(v);
      decltype(auto) old_value = value_.exchange(validated_value);
      if(old_value != validated_value)
      {
        emit<setting_signal_id::value_changed>();
      }
      return validated_value == v;
    },
    [&](const setting_validator_list<T>::sptr_type& validator_list) -> bool
    {
      const auto contains = validator_list->contains(v);
      if(contains)
      {
        decltype(auto) old_value = value_.exchange(v);
        if(old_value != v)
        {
          emit<setting_signal_id::value_changed>();
        }
      }
      return contains;
    }
  );
}

} // namespace bibstd::framework
