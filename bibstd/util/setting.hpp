#pragma once

#include "util/property.hpp"
#include "util/setting_common.hpp"
#include "util/setting_validator.hpp"
#include "util/visit_helper.hpp"

#include <memory>

namespace bibstd::util
{

///
/// Setting class.
///
template<underlying_setting_type T>
class setting final
{
public: // Typedefs
  using value_type = T;
  using sptr_type = std::shared_ptr<setting<T>>;

public: // Structors
  setting(
    const std::string& parent,
    const std::string& name,
    util::property<T>&& value,
    setting_validator<T>&& validator = setting_validator_unbound{}
  );

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
  const std::string parent;
  const std::string name;
  const setting_validator<T> validator;

private: // Variables
  util::property<T> value_;
};

///
///
template<underlying_setting_type T>
setting<T>::setting(
  const std::string& parent_, const std::string& name_, util::property<T>&& value, setting_validator<T>&& validator_
)
  : parent{parent_}
  , name{name_}
  , validator{std::move(validator_)}
  , value_{std::move(value)}
{
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
    [&](const setting_validator_unbound&) -> bool
    {
      value_.value(v);
      return true;
    },
    [&](const setting_validator_const_range<T>& validator_range) -> bool
    {
      const auto validated_value = validator_range.validate(v);
      value_.value(validated_value);
      return validated_value == v;
    },
    [&](const setting_validator_const_list<T>& validator_list) -> bool
    {
      const auto contains = validator_list.contains(v);
      if(contains)
      {
        value_.value(v);
      }
      return contains;
    }
  );
}

} // namespace bibstd::util
