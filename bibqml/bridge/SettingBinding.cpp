#include "bibqml/bridge/SettingBinding.hpp"
#include "bibqml/bridge/BridgeSettings.hpp"
#include "bibqml/util/SettingValue.hpp"

#include <bibstd/util/exception.hpp>
#include <bibstd/util/log.hpp>

#include <QColor>
#include <QJSValue>
#include <QMetaObject>
#include <QMetaType>

#include <cstdint>
#include <optional>
#include <variant>

namespace bibqml
{
namespace
{

///
/// Deduce the default value of a new setting from a QML provided value. The alternative of the
/// returned variant defines the value type a setting declared in QML is created with. Reading
/// and writing an existing setting is defined by the value type of that setting.
/// \return default setting value, std::nullopt if the value type is not supported
///
auto toDefaultSettingValue(const QVariant& variant) -> std::optional<bibstd::framework::setting_type_erased_variant>
{
  // QML might provide special java script types that do not belong to a non user meta type.
  const auto value = [&]
  {
    const auto typeId = variant.metaType().id();
    if(typeId >= QMetaType::User && variant.canConvert<QJSValue>())
    {
      // came through as a JS array wrapped in QJSValue — unwrap it properly
      return variant.value<QJSValue>().toVariant();
    }
    return variant;
  }();

  switch(value.metaType().id())
  {
  case QMetaType::Bool: return value.toBool();
  case QMetaType::Int: [[fallthrough]];
  case QMetaType::UInt: [[fallthrough]];
  case QMetaType::LongLong: [[fallthrough]];
  case QMetaType::ULongLong: return static_cast<std::int64_t>(value.toLongLong());
  case QMetaType::Float: [[fallthrough]];
  case QMetaType::Double: return value.toDouble();
  case QMetaType::QString: return value.toString().toStdString();
  case QMetaType::QColor: return value.value<QColor>().name(QColor::HexArgb).toStdString();
  default: return std::nullopt;
  }
}

} // anonymous namespace

///
///
SettingBinding::SettingBinding(const bibstd::util::non_owning_ptr<QObject> parent)
  : QObject{parent}
{
}

///
///
SettingBinding::~SettingBinding() noexcept = default;

///
///
void SettingBinding::classBegin()
{
}

///
///
void SettingBinding::componentComplete()
{
  complete_ = true;
  bind();
}

///
///
QString SettingBinding::path() const
{
  return path_;
}

///
///
QVariant SettingBinding::defaultValue() const
{
  return defaultValue_;
}

///
///
QVariant SettingBinding::value() const
{
  return setting_ ? toQmlValue(*setting_) : defaultValue_;
}

///
///
bool SettingBinding::bound() const
{
  return setting_.has_value();
}

///
///
void SettingBinding::setPath(const QString& path)
{
  if(path_ == path)
  {
    return;
  }
  if(complete_)
  {
    LOG_ERROR("set path failed: the path is constant: path=\"{}\"", path_.toStdString());
    return;
  }
  path_ = path;
  emit pathChanged();
}

///
///
void SettingBinding::setDefaultValue(const QVariant& value)
{
  if(defaultValue_ == value)
  {
    return;
  }
  if(complete_)
  {
    LOG_ERROR("set default value failed: the default value is constant: path=\"{}\"", path_.toStdString());
    return;
  }
  defaultValue_ = value;
  emit defaultValueChanged();
  // The default value is only used to create the setting. As long as this
  // binding is not bound, the default value is reported as value.
  emit valueChanged();
}

///
///
void SettingBinding::setValue(const QVariant& value)
{
  if(!setting_)
  {
    LOG_ERROR("write setting value failed: binding is not bound: path=\"{}\"", path_.toStdString());
    return;
  }
  try
  {
    // The conversion of the value is defined by the value type of the setting.
    const auto isValueSet = setQmlValue(*setting_, value);
    if(!isValueSet)
    {
      // The value was rejected. Notify the QML layer such that it can reset to the current value.
      // If the setting changed its value, a second notification follows.
      emit valueChanged();
    }
  }
  catch(...)
  {
    LOG_ERROR("exception writing setting value: {}", bibstd::util::exception_report());
    emit valueChanged();
  }
}

///
///
void SettingBinding::bind()
{
  // Since path and default value are constant, this binds exactly once.
  if(path_.isEmpty())
  {
    LOG_ERROR("bind setting failed: no path provided");
    return;
  }
  const auto registry = BridgeSettings::instance();
  if(!registry)
  {
    LOG_ERROR("bind setting failed: settings registry does not exist: path=\"{}\"", path_.toStdString());
    return;
  }
  const auto& workflowSettings = registry->workflowSettings();
  const auto defaultSettingValue = toDefaultSettingValue(defaultValue_);
  if(!defaultSettingValue)
  {
    LOG_ERROR("bind setting failed: unsupported default value type: path=\"{}\"", path_.toStdString());
    return;
  }

  try
  {
    setting_ = std::visit(
      [&](const auto& defaultValue) { return workflowSettings->type_erased_setting(path_.toStdString(), defaultValue); },
      *defaultSettingValue
    );
  }
  catch(...)
  {
    // Without a setting the default value stays the value of this binding.
    LOG_ERROR("bind setting failed: {}", bibstd::util::exception_report());
    return;
  }

  std::visit(
    [this](const auto setting)
    {
      setting->signal_adapter.connect_queued(
        &bibstd::framework::setting_signals::value_changed,
        [this]() { QMetaObject::invokeMethod(this, [this]() { emit valueChanged(); }, Qt::QueuedConnection); },
        executor_
      );
    },
    *setting_
  );
  LOG_DEBUG("bind setting: path=\"{}\"", path_.toStdString());
  emit boundChanged();
  emit valueChanged();
}

} // namespace bibqml
