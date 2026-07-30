#include "bibqml/bridge/BridgeSettings.hpp"

#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <utility>

namespace bibqml
{
namespace
{

// Instance the QML layer operates on. It is registered by the constructor of the
// application owned instance and reset when that instance is destroyed.
bibstd::util::non_owning_ptr<BridgeSettings> instance_{nullptr};

} // anonymous namespace

///
///
bibstd::util::non_owning_ptr<BridgeSettings> BridgeSettings::instance()
{
  return instance_;
}

///
///
BridgeSettings* BridgeSettings::create([[maybe_unused]] QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
  if(!instance_)
  {
    LOG_ERROR("settings registry does not exist: settings cannot be bound from qml");
    return nullptr;
  }
  // The instance is owned by the application, the QML engine must not delete it.
  if(jsEngine)
  {
    QJSEngine::setObjectOwnership(instance_, QJSEngine::CppOwnership);
  }
  return instance_;
}

///
///
BridgeSettings::BridgeSettings(
  std::shared_ptr<bibstd::workflow::workflow_settings> workflowSettings, const bibstd::util::non_owning_ptr<QObject> parent
)
  : QObject{parent}
  , workflowSettings_{std::move(workflowSettings)}
{
  if(instance_)
  {
    LOG_WARN("settings registry already exists: qml will operate on the first instance");
    return;
  }
  instance_ = this;
}

///
///
BridgeSettings::~BridgeSettings() noexcept
{
  if(instance_ == this)
  {
    instance_ = nullptr;
  }
}

///
///
auto BridgeSettings::workflowSettings() const -> const std::shared_ptr<bibstd::workflow::workflow_settings>&
{
  return workflowSettings_;
}

///
///
bool BridgeSettings::contains(const QString& path) const
{
  return workflowSettings_ && workflowSettings_->type_erased_setting(path.toStdString()).has_value();
}

} // namespace bibqml
