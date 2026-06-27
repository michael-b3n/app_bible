#pragma once

#include "src/construct_backend.hpp"

#include <QGuiApplication>
#include <QQmlApplicationEngine>

// Forward declarations
namespace bibqml
{
class BridgeBibleRefOcr;
class ScriptureListModel;
class SettingsListModel;
} // namespace bibqml

namespace aba
{

struct bridge_instance final
{
  // Structors
  ~bridge_instance() noexcept;

  // Variables
  std::unique_ptr<bibqml::SettingsListModel> settings_list_model;
  std::unique_ptr<bibqml::BridgeBibleRefOcr> bridge_bible_ref_ocr;
  std::unique_ptr<bibqml::ScriptureListModel> scripture_list_model;
};

///
/// Disconnect all signal connections and perform necessary cleanup for the bridge instance.
/// This will stop the frontend backend communication.
/// \param instance The bridge instance to disconnect
///
auto disconnect_bridge(bridge_instance& instance) -> void;

///
/// Initialize the bridge instances.
/// \param app The QGuiApplication instance
/// \param backend The backend instance
/// \return bridge instance
///
auto construct_bridge(QGuiApplication& app, backend_instance& backend) -> bridge_instance;

///
/// Connect internal signals between bridge components.
/// \param instance The bridge instance to connect
///
auto connect_bridge(bridge_instance& instance) -> void;

///
/// Connect the QML engine to the bridge instances and load the main QML file.
/// \param engine The QQmlApplicationEngine instance
/// \param app The QGuiApplication instance
/// \param bridge The bridge instance
///
auto connect_engine(QQmlApplicationEngine& engine, QGuiApplication& app, bridge_instance& bridge) -> void;

} // namespace aba
