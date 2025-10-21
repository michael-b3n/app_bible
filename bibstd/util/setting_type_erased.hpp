#pragma once

#include "util/non_owning_ptr.hpp"
#include "util/setting.hpp"

#include <functional>
#include <memory>

namespace bibstd::util
{

///
/// Type erased setting class. This class wraps the general setting interface.
/// `This` must be destroyed before the underlying setting is destroyed.
///
template<erased_setting_type T>
class setting_type_erased final
{
public: // Typedefs
  using value_type = T;

public: // Structors
  template<underlying_setting_type U>
  setting_type_erased(const std::shared_ptr<setting<U>>& setting);

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
  const std::function<T()> get_;
  const std::function<bool(const T&)> set_;
};

///
/// Deduction guide for setting_type_erased.
///
template<underlying_setting_type U>
setting_type_erased(const std::shared_ptr<setting<U>>& setting) -> setting_type_erased<erased_setting_type_from<U>>;

namespace detail
{

///
/// Convert setting validator to type erased setting validator.
/// \param validator setting validator that shall be converted
/// \return type erased setting validator
///
template<underlying_setting_type U>
auto validator_type_erased(const setting_validator<U>& validator) -> setting_validator<erased_setting_type_from<U>>
{
  using retrun_type = setting_validator<erased_setting_type_from<U>>;
  if constexpr(erased_setting_type<U>)
  {
    return validator;
  }
  else {
    return util::visit_lambdas(
      validator,
      [&]([[maybe_unused]] const setting_validator_unbound&) -> retrun_type { return setting_validator_unbound{}; },
      [&](const setting_validator_const_range<U>& validator_range) -> retrun_type
      { return static_cast<setting_validator_const_range<erased_setting_type_from<U>>>(validator_range); },
      [&](const setting_validator_const_list<U>& validator_list) -> retrun_type
      { return static_cast<setting_validator_const_list<erased_setting_type_from<U>>>(validator_list); }
    );
  }
}

} // namespace detail

///
///
template<erased_setting_type T>
template<underlying_setting_type U>
setting_type_erased<T>::setting_type_erased(const std::shared_ptr<setting<U>>& setting)
  : parent{setting->parent}
  , name{setting->name}
  , validator{detail::validator_type_erased(setting->validator)}
  , get_{[setting, converter = create_setting_converter<U, T>()]() { return converter(setting->value()); }}
  , set_{[setting, converter = create_setting_converter<T, U>()](const T& v) { return setting->value(converter(v)); }}
{
}

///
///
template<erased_setting_type T>
auto setting_type_erased<T>::value() const -> T
{
  return get_();
}

///
///
template<erased_setting_type T>
auto setting_type_erased<T>::value(const T& v) -> bool
{
  return set_(v);
}

} // namespace bibstd::util
