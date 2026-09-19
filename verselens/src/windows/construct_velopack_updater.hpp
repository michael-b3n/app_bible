#pragma once

#include "src/construct_bridge.hpp"
#include "src/construct_translations.hpp"
#include "src/windows/app_velopack_updater.hpp"

#include <QGuiApplication>

#include <memory>

namespace verselens
{

///
/// Start the updater. The qml layer asks for checks and for the install of a downloaded update. The install
/// quits the application, Velopack starts it again once the update is in place.
/// \return updater, it has to be destroyed before the bridge
///
[[nodiscard]] auto construct_velopack_updater(
  QGuiApplication& app, bridge_instance& bridge, translations_instance& translations
) -> std::unique_ptr<app_velopack_updater>;

} // namespace verselens
