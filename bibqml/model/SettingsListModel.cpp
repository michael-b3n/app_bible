#include "bibqml/model/SettingsListModel.hpp"

#include <bibstd/framework/setting.hpp>
#include <bibstd/framework/setting_type_erased.hpp>
#include <bibstd/framework/setting_validator.hpp>
#include <bibstd/math/arithmetic.hpp>
#include <bibstd/meta/chrono.hpp>
#include <bibstd/meta/type_traits.hpp>
#include <bibstd/util/exception.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/numeric_cast.hpp>
#include <bibstd/util/visit_helper.hpp>

#include <algorithm>
#include <chrono>
#include <concepts>
#include <optional>
#include <ranges>
#include <type_traits>

namespace bibqml
{
using std::remove_cvref_t;

// Typedefs
template<typename T>
using setting_ptr = bibstd::util::non_owning_ptr<bibstd::framework::setting_type_erased<T>>;
// clang-format off
template<typename T>
struct setting_value_type { using type = T; };
template<typename T>
struct setting_value_type<std::optional<T>> { using type = T; };
template<typename T>
struct setting_value_type<std::vector<T>> { using type = T; };
// clang-format on
template<typename T>
using setting_raw_value_type = typename setting_value_type<typename std::remove_pointer_t<T>::value_type>::type;
template<typename T>
inline constexpr bool always_false_v = false;
template<typename T>
using validator_range_sptr = bibstd::framework::setting_validator_range_type_erased<T>::sptr_type;
template<typename T>
using validator_list_sptr = bibstd::framework::setting_validator_list_type_erased<T>::sptr_type;
using validator_unbound_sptr = bibstd::framework::setting_validator_unbound::sptr_type;

///
/// Protected numeric cast between integers.
/// \return optional casted integer, std::nullopt if cast fails
///
template<std::integral T>
auto integer_cast(const std::integral auto value) -> std::optional<T>
{
  try
  {
    const auto v = numeric_cast<T>(value);
    return v;
  }
  catch(...)
  {
    LOG_ERROR("bad numeric cast to {}:  {}", typeid(T).name(), bibstd::util::exception_report());
    return std::nullopt;
  }
}

///
/// Report an unexpected type mismatch and return false.
/// \return false in any case needed for setValue overload
///
auto throw_type_mismatch_error(const auto setting, const auto& value) -> bool
{
  using s_type = typename std::remove_pointer_t<std::remove_const_t<decltype(setting)>>::value_type;
  using v_type = std::remove_cvref_t<decltype(value)>;
  throw bibstd::util::exception{
    std::format("type mismatch: setting_type=\"{}\", value_type=\"{}\"", typeid(s_type).name(), typeid(v_type).name())
  };
  return false;
}

///
/// Deduce the value type enum value from the settings pointer.
/// \return enum value describing the value type
///
constexpr auto getValueType(const auto& setting) -> SettingsListModel::ValueType
{
  using value_type = setting_raw_value_type<std::remove_cvref_t<decltype(setting)>>;
  // clang-format off
  if constexpr(std::is_same_v<value_type, bool>) { return SettingsListModel::ValueType::BoolValueType; }
  else if constexpr(std::is_same_v<value_type, std::int32_t>) { return SettingsListModel::ValueType::IntValueType; }
  else if constexpr(std::is_same_v<value_type, std::int64_t>) { return SettingsListModel::ValueType::IntValueType; }
  else if constexpr(std::is_same_v<value_type, std::uint32_t>) { return SettingsListModel::ValueType::IntValueType; }
  else if constexpr(std::is_same_v<value_type, std::uint64_t>) { return SettingsListModel::ValueType::IntValueType; }
  else if constexpr(std::is_same_v<value_type, double>) { return SettingsListModel::ValueType::DoubleValueType; }
  else if constexpr(std::is_same_v<value_type, std::string>) { return SettingsListModel::ValueType::StringValueType; }
  else if constexpr(std::is_same_v<value_type, std::chrono::milliseconds>) { return SettingsListModel::ValueType::TimeValueType; }
  else if constexpr(std::is_same_v<value_type, std::chrono::seconds>) { return SettingsListModel::ValueType::TimeValueType; }
  else if constexpr(std::is_same_v<value_type, std::chrono::minutes>) { return SettingsListModel::ValueType::TimeValueType; }
  else if constexpr(std::is_same_v<value_type, std::filesystem::path>) { return SettingsListModel::ValueType::PathValueType; }
  else { static_assert(always_false_v<value_type>, "Unsupported setting value type"); }
  // clang-format on
}

///
/// Deduce the wrapper type enum value from the settings pointer.
/// \return enum value describing the wrapper type
///
constexpr auto getWrapperType(const auto& setting) -> SettingsListModel::WrapperType
{
  using value_type = std::remove_pointer_t<std::remove_cvref_t<decltype(setting)>>::value_type;
  using raw_value_type = setting_raw_value_type<std::remove_cvref_t<decltype(setting)>>;
  static_assert(std::is_same_v<std::remove_cvref_t<value_type>, value_type>);
  static_assert(std::is_same_v<std::remove_cvref_t<raw_value_type>, raw_value_type>);

  // clang-format off
  if constexpr(std::is_same_v<value_type, raw_value_type>) { return SettingsListModel::WrapperType::NoneWrapperType; }
  else if constexpr(std::is_same_v<value_type, std::optional<raw_value_type>>) { return SettingsListModel::WrapperType::OptionalWrapperType; }
  else if constexpr(std::is_same_v<value_type, std::vector<raw_value_type>>) { return SettingsListModel::WrapperType::ListWrapperType; }
  else { static_assert(always_false_v<value_type>, "Unsupported setting wrapper type"); }
  // clang-format on
}

///
/// Deduce the validator type enum value from the settings validator variant.
/// \return enum value describing the validator type
///
auto getValidatorType(const auto& setting) -> SettingsListModel::ValidatorType
{
  using value_type = std::remove_pointer_t<std::remove_cvref_t<decltype(setting)>>::value_type;
  return bibstd::util::visit_lambdas(
    setting->validator,
    []([[maybe_unused]] const validator_unbound_sptr&) { return SettingsListModel::ValidatorType::UnboundValidatorType; },
    []([[maybe_unused]] const validator_range_sptr<value_type>&)
    { return SettingsListModel::ValidatorType::RangeValidatorType; },
    []([[maybe_unused]] const validator_list_sptr<value_type>&) { return SettingsListModel::ValidatorType::ListValidatorType; }
  );
}

///
/// \return QVariant converted from a boolean
///
auto toVariant(const bool& value) -> QVariant
{
  return QVariant(value);
}

///
/// \return QVariant converted from an integral
///
template<std::integral T>
auto toVariant(const T value) -> QVariant
{
  const auto v = integer_cast<int>(value);
  return v ? QVariant(*v) : QVariant{};
}

///
/// \return QVariant converted from a double
///
auto toVariant(const double& value) -> QVariant
{
  return QVariant(value);
}

///
/// \return QVariant containing double type converted from a duration
///
template<typename T>
  requires(bibstd::meta::is_duration_v<T>)
auto toVariant(const T& value) -> QVariant
{
  return QVariant(std::chrono::duration<double>(value).count());
}

///
/// \return QVariant containing QString converted from a std::string
///
auto toVariant(const std::string& value) -> QVariant
{
  return QString::fromStdString(value);
}

///
/// \return QVariant containing QString converted from a path
///
auto toVariant(const std::filesystem::path& value) -> QVariant
{
  return QString::fromStdString(value.string());
}

///
/// \return QVariant with a value or empty converted from a std::optional
///
template<typename T>
auto toVariant(const std::optional<T>& value) -> QVariant
{
  return value ? toVariant(*value) : QVariant{};
}

///
/// \return QVariant containing QVariantList converted from a std::vector type
///
template<typename T>
auto toVariant(const std::vector<T>& list) -> QVariant
{
  QList<QVariant> result;
  result.reserve(static_cast<int>(list.size()));
  std::ranges::for_each(list, [&](const auto& item) { result.emplace_back(toVariant(item)); });
  return result;
}

///
/// Deduce the validator type enum value from the settings validator variant.
/// \return enum value describing the validator type
///
auto getlistValidatorData(const auto& setting) -> QList<QVariant>
{
  using value_type = std::remove_pointer_t<std::remove_cvref_t<decltype(setting)>>::value_type;
  return bibstd::util::visit_lambdas(
    setting->validator,
    []([[maybe_unused]] const validator_unbound_sptr&) { return QList<QVariant>{}; },
    []([[maybe_unused]] const validator_range_sptr<value_type>&) { return QList<QVariant>{}; },
    [](const validator_list_sptr<value_type>& v)
    {
      const auto available = v->available();
      QList<QVariant> result;
      result.reserve(static_cast<int>(available.size()));
      std::ranges::for_each(available, [&](const auto& item) { result.push_back(toVariant(item)); });
      return result;
    }
  );
}

///
///
auto setValueConvertible(const auto setting, const auto& value) -> bool
{
  if constexpr(std::is_convertible_v<std::remove_cvref_t<decltype(value)>, setting_raw_value_type<decltype(setting)>>)
  {
    return setting->value(value);
  }
  else
  {
    return throw_type_mismatch_error(setting, value);
  }
}

///
///
auto setValueConvertibleVec(const auto setting, const auto& value) -> bool
{
  using vector_value_type = bibstd::meta::remove_wrapper_t<decltype(value)>;
  static_assert(std::is_same_v<std::vector<vector_value_type>, std::remove_cvref_t<decltype(value)>>);
  if constexpr(std::is_same_v<vector_value_type, setting_raw_value_type<decltype(setting)>>)
  {
    return setting->value(value);
  }
  else if constexpr(std::is_convertible_v<vector_value_type, setting_raw_value_type<decltype(setting)>>)
  {
    return setting->value(
      value | std::views::transform([&](const auto& v) { return static_cast<setting_raw_value_type<decltype(setting)>>(v); }) |
      std::ranges::to<std::vector>()
    );
  }
  else
  {
    return throw_type_mismatch_error(setting, value);
  }
}

///
///
auto setValueInteger(const auto setting, const auto& value) -> bool
{
  static_assert(std::integral<setting_raw_value_type<decltype(setting)>>);
  if constexpr(std::integral<decltype(value)>)
  {
    const auto v = integer_cast<setting_raw_value_type<decltype(setting)>>(value);
    return v ? setting->value(*v) : false;
  }
  else
  {
    return throw_type_mismatch_error(setting, value);
  }
}

///
///
auto setValueIntegerVec(const auto setting, const auto& value) -> bool
{
  static_assert(std::integral<setting_raw_value_type<decltype(setting)>>);
  using vector_value_type = bibstd::meta::remove_wrapper_t<decltype(value)>;
  static_assert(std::is_same_v<std::vector<vector_value_type>, std::remove_cvref_t<decltype(value)>>);
  if constexpr(std::is_same_v<vector_value_type, setting_raw_value_type<decltype(setting)>>)
  {
    return setting->value(value);
  }
  else if constexpr(std::integral<vector_value_type>)
  {
    auto dest = std::vector<setting_raw_value_type<decltype(setting)>>{};
    dest.reserve(value.size());
    auto result = std::ranges::all_of(
      value,
      [&](const auto v)
      {
        if(const auto i = integer_cast<setting_raw_value_type<decltype(setting)>>(v))
        {
          dest.push_back(*i);
          return true;
        }
        return false;
      }
    );
    if(result)
    {
      result = setting->value(dest);
    }
    return result;
  }
  else
  {
    return throw_type_mismatch_error(setting, value);
  }
}

///
///
auto setValueDuration(const auto setting, const auto value) -> bool
{
  static_assert(bibstd::meta::is_duration_v<setting_raw_value_type<decltype(setting)>>);
  if constexpr(std::floating_point<decltype(value)>)
  {
    const auto d = std::chrono::duration_cast<setting_raw_value_type<decltype(setting)>>(std::chrono::duration<double>(value));
    return setting->value(d);
  }
  else
  {
    return throw_type_mismatch_error(setting, value);
  }
}

///
///
auto setValueDurationVec(const auto setting, const auto& value) -> bool
{
  using vector_value_type = bibstd::meta::remove_wrapper_t<decltype(value)>;
  static_assert(std::is_same_v<std::vector<vector_value_type>, std::remove_cvref_t<decltype(value)>>);
  if constexpr(std::floating_point<vector_value_type>)
  {
    return setting->value(
      value |
      std::views::transform(
        [&](const auto v)
        { return std::chrono::duration_cast<setting_raw_value_type<decltype(setting)>>(std::chrono::duration<double>(v)); }
      ) |
      std::ranges::to<std::vector>()
    );
  }
  else
  {
    return throw_type_mismatch_error(setting, value);
  }
}

///
///
SettingsListModel::SettingsListModel(
  std::shared_ptr<bibstd::workflow::workflow_settings> workflow_settings, const bibstd::util::non_owning_ptr<QObject> parent
)
  : QAbstractListModel{parent}
  , workflow_settings_{std::move(workflow_settings)}
{
  std::ranges::for_each(
    workflow_settings_->type_erased_settings(),
    [&](const auto& setting_data)
    {
      std::visit(
        [&](const auto setting)
        {
          addEntry(setting_data.path, setting);
          const auto row_index = bibstd::math::arithmetic::subtract(entries_.size(), decltype(entries_.size()){1}).value();
          setting->signal_adapter.connect_queued(
            &bibstd::framework::setting_signals::value_changed,
            [this, row_index]()
            {
              QMetaObject::invokeMethod(
                this,
                [this, row_index]() { emit dataChanged(index(row_index), index(row_index), {Role::ValueRole}); },
                Qt::QueuedConnection
              );
            },
            executor_
          );
          setting->signal_adapter.connect_queued(
            &bibstd::framework::setting_signals::validator_changed,
            [this, setting, row_index]()
            {
              using value_type = std::remove_pointer_t<std::remove_cvref_t<decltype(setting)>>::value_type;
              bibstd::util::visit_lambdas(
                setting->validator,
                []([[maybe_unused]] const validator_unbound_sptr&) { /*noop*/ },
                []([[maybe_unused]] const validator_range_sptr<value_type>&) { /*noop*/ },
                [this, row_index]([[maybe_unused]] const validator_list_sptr<value_type>&)
                {
                  QMetaObject::invokeMethod(
                    this,
                    [this, row_index]()
                    { emit dataChanged(index(row_index), index(row_index), {Role::ListValidatorDataRole}); },
                    Qt::QueuedConnection
                  );
                }
              );
            },
            executor_
          );
        },
        setting_data.setting
      );
    }
  );
}

///
///
SettingsListModel::~SettingsListModel() noexcept = default;

///
///
int SettingsListModel::rowCount(const QModelIndex& parent) const
{
  if(parent.isValid())
  {
    return 0;
  }
  return static_cast<int>(entries_.size());
}

///
///
QVariant SettingsListModel::data(const QModelIndex& index, const int role) const
{
  if(!index.isValid() || index.row() < 0 || static_cast<decltype(entries_.size())>(index.row()) >= entries_.size())
  {
    return {};
  }
  [[maybe_unused]] const auto& entry = entries_.at(static_cast<std::size_t>(index.row()));
  switch(role)
  {
  case PathRole: return toVariant(entry.path);
  case ValueTypeRole: return static_cast<int>(entry.valueType);
  case WrapperTypeRole: return static_cast<int>(entry.wrapperType);
  case ValidatorTypeRole: return static_cast<int>(entry.validatorType);
  case ValueRole: return std::visit([&](const auto s) { return toVariant(s->value()); }, entry.setting);
  case ListValidatorDataRole: return std::visit([&](const auto s) { return getlistValidatorData(s); }, entry.setting);
  default: return {};
  }
}

///
///
QHash<int, QByteArray> SettingsListModel::roleNames() const
{
  return {
    {             PathRole,              "path"},
    {        ValueTypeRole,         "valueType"},
    {      WrapperTypeRole,       "wrapperType"},
    {    ValidatorTypeRole,     "validatorType"},
    {            ValueRole,             "value"},
    {ListValidatorDataRole, "listValidatorData"},
  };
}

///
///
bool SettingsListModel::setData(const QModelIndex& index, const QVariant& value, const int role)
{
  if(!index.isValid() || index.row() < 0 || static_cast<decltype(entries_.size())>(index.row()) >= entries_.size())
  {
    return false;
  }
  [[maybe_unused]] const auto& entry = entries_.at(static_cast<std::size_t>(index.row()));

  const auto set_plain = [&entry](const auto& v) -> bool
  {
    return bibstd::util::visit_lambdas(
      entry.setting,
      [&](const setting_ptr<bool> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::int32_t> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::int64_t> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::uint32_t> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::uint64_t> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<double> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::string> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::chrono::milliseconds> setting) { return setValueDuration(setting, v); },
      [&](const setting_ptr<std::chrono::seconds> setting) { return setValueDuration(setting, v); },
      [&](const setting_ptr<std::chrono::minutes> setting) { return setValueDuration(setting, v); },
      [&](const setting_ptr<std::filesystem::path> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::optional<bool>> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::optional<std::int32_t>> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::optional<std::int64_t>> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::optional<std::uint32_t>> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::optional<std::uint64_t>> setting) { return setValueInteger(setting, v); },
      [&](const setting_ptr<std::optional<double>> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::optional<std::string>> setting) { return setValueConvertible(setting, v); },
      [&](const setting_ptr<std::optional<std::chrono::milliseconds>> setting) { return setValueDuration(setting, v); },
      [&](const setting_ptr<std::optional<std::chrono::seconds>> setting) { return setValueDuration(setting, v); },
      [&](const setting_ptr<std::optional<std::chrono::minutes>> setting) { return setValueDuration(setting, v); },
      [&](const setting_ptr<std::optional<std::filesystem::path>> setting) { return setValueConvertible(setting, v); },
      /*vectors types not supported*/ [&](const auto setting) { return throw_type_mismatch_error(setting, v); }
    );
  };

  const auto set_vec = [&entry](const auto& v) -> bool
  {
    return bibstd::util::visit_lambdas(
      entry.setting,
      /*plain and optional types not supported*/ [&](const auto setting) { return throw_type_mismatch_error(setting, v); },
      [&](const setting_ptr<std::vector<std::int32_t>> setting) { return setValueIntegerVec(setting, v); },
      [&](const setting_ptr<std::vector<std::int64_t>> setting) { return setValueIntegerVec(setting, v); },
      [&](const setting_ptr<std::vector<std::uint32_t>> setting) { return setValueIntegerVec(setting, v); },
      [&](const setting_ptr<std::vector<std::uint64_t>> setting) { return setValueIntegerVec(setting, v); },
      [&](const setting_ptr<std::vector<double>> setting) { return setValueConvertibleVec(setting, v); },
      [&](const setting_ptr<std::vector<std::string>> setting) { return setValueConvertibleVec(setting, v); },
      [&](const setting_ptr<std::vector<std::chrono::milliseconds>> setting) { return setValueDurationVec(setting, v); },
      [&](const setting_ptr<std::vector<std::chrono::seconds>> setting) { return setValueDurationVec(setting, v); },
      [&](const setting_ptr<std::vector<std::chrono::minutes>> setting) { return setValueDurationVec(setting, v); },
      [&](const setting_ptr<std::vector<std::filesystem::path>> setting) { return setValueConvertibleVec(setting, v); }
    );
  };

  const auto set_null = [&entry]() -> bool
  {
    return std::visit(
      [&](const auto setting)
      {
        using value_type = std::remove_pointer_t<std::remove_cvref_t<decltype(setting)>>::value_type;
        using raw_value_type = setting_raw_value_type<std::remove_cvref_t<decltype(setting)>>;
        static_assert(std::is_same_v<std::remove_cvref_t<value_type>, value_type>);
        static_assert(std::is_same_v<std::remove_cvref_t<raw_value_type>, raw_value_type>);
        if constexpr(std::is_same_v<value_type, raw_value_type>)
        {
          LOG_ERROR("type mismatch: failed to set null value on plain types");
          return false;
        }
        else
        {
          return setting->value({});
        }
      },
      entry.setting
    );
  };

  switch(role)
  {
  case PathRole: return false;          // immutable
  case ValueTypeRole: return false;     // immutable
  case WrapperTypeRole: return false;   // immutable
  case ValidatorTypeRole: return false; // immutable
  case ValueRole:
  {
    try
    {
      const auto is_value_set = [&]
      {
        const auto typeId = value.metaType().id();
        switch(typeId)
        {
        case QMetaType::Bool: return set_plain(value.toBool());
        case QMetaType::Int: return set_plain(value.toInt());
        case QMetaType::Double: return set_plain(value.toDouble());
        case QMetaType::QString: return set_plain(value.toString().toStdString());
        case QMetaType::QVariantList:
        {
          const auto& list = value.toList();
          const auto to_vector = [&](auto&& converter)
          {
            auto vec = std::vector<std::invoke_result_t<decltype(converter), QVariant>>(list.size());
            std::ranges::for_each(list, [&](const auto& variant) { vec.emplace_back(converter(variant)); });
            return vec;
          };
          if(!list.empty())
          {
            const auto frontTypeId = list.front().metaType().id();
            switch(frontTypeId)
            {
              // boolean not supported due to forbidden std::vector<bool>
            case QMetaType::Int: return set_vec(to_vector([](const auto& v) { return v.toInt(); }));
            case QMetaType::Double: return set_vec(to_vector([](const auto& v) { return v.toDouble(); }));
            case QMetaType::QString: return set_vec(to_vector([](const auto& v) { return v.toString().toStdString(); }));
            }
          }
          return set_null();
        }
        default: return set_null();
        };
      }();
      if(!is_value_set)
      {
        // Emit dataChanged signal to notify that the value
        // could not be set such that the UI can be reset.
        emit dataChanged(index, index, {role});
      }
      return is_value_set;
    }
    catch(...)
    {
      LOG_ERROR("exception writing setting value: {}", bibstd::util::exception_report());
      emit dataChanged(index, index, {role});
      return false;
    }
  }
  case ListValidatorDataRole: return false; // immutable
  default: return false;
  }
}

///
///
Qt::ItemFlags SettingsListModel::flags(const QModelIndex& index) const
{
  return Qt::NoItemFlags;
}

///
///
void SettingsListModel::disconnect()
{
  executor_.disconnect();
}

///
///
void SettingsListModel::addEntry(std::string path, const auto& setting)
{
  entries_.emplace_back(
    Entry{
      .path = std::move(path),
      .valueType = getValueType(setting),
      .wrapperType = getWrapperType(setting),
      .validatorType = getValidatorType(setting),
      .setting = setting
    }
  );
}

} // namespace bibqml
