#include "bibqml/bridge/BridgeScripture.hpp"

#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

#include <QMetaObject>

namespace bibqml
{

///
///
BridgeScripture::BridgeScripture(std::shared_ptr<bibstd::workflow::workflow_scripture> workflow_scripture, QObject* parent)
  : QObject{parent}
  , workflow_scripture_{std::move(workflow_scripture)}
{
}

///
///
BridgeScripture::~BridgeScripture() noexcept = default;

///
///
auto BridgeScripture::disconnect() -> void
{
  executor_.disconnect();
}

} // namespace bibqml
