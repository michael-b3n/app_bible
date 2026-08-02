#include "Icons.hpp"

namespace aba::qml
{
namespace detail
{

///
///
auto toIconUrl(std::string_view name) -> QString
{
  static constexpr auto prefix = std::string_view{"qrc:/qt/qml/ui/qml/res/"};
  return QString::fromLatin1(prefix.data(), prefix.size()).append(QLatin1StringView(name.data(), name.size()));
}

} // namespace detail

///
///
Icons::Icons(bibstd::util::non_owning_ptr<QObject> parent)
  : QObject{parent}
{
}

///
///
Icons::~Icons() noexcept = default;

} // namespace aba::qml
