#include "Metrics.hpp"

namespace verselens::qml
{

///
///
Metrics::Metrics(bibstd::util::non_owning_ptr<QObject> parent)
  : QObject{parent}
{
}

///
///
Metrics::~Metrics() noexcept = default;

} // namespace verselens::qml
