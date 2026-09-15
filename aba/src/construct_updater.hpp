#pragma once

#include "src/app_updater.hpp"
#include "src/construct_bridge.hpp"
#include "src/construct_translations.hpp"

#include <QGuiApplication>

#include <memory>

namespace aba
{

///
/// Start the updater. A downloaded update is reported to the qml layer, which asks for its install. The install
/// quits the application, Velopack starts it again once the update is in place.
/// \return updater, it has to be destroyed before the bridge
///
[[nodiscard]] auto construct_updater(QGuiApplication& app, bridge_instance& bridge, translations_instance& translations)
  -> std::unique_ptr<updater>;

} // namespace aba
