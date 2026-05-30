#include "Metrics.hpp"

namespace aba::qml
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

} // namespace aba::qml
