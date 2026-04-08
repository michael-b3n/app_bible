#pragma once

#include "src/construct_backend.hpp"

#include <bibstd/util/scope_guard.hpp>
#include <bibstd/workflow/workflow_base.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

namespace bibqml
{
// Forward declarations
class BridgeBibleRefOcr;
} // namespace bibqml
namespace aba
{

struct bridge_instance final
{
  // Variables
  std::unique_ptr<bibqml::BridgeBibleRefOcr> bridge_bible_ref_ocr;
};

///
/// Disconnect all signal connections and perform necessary cleanup for the bridge instance.
/// This will stop the frontend backend communication.
/// \param instance The bridge instance to disconnect
///
auto disconnect_bridge(bridge_instance& instance) -> void;

///
/// Initialize backend components.
/// \return backend instance
///
auto construct_bridge(QGuiApplication& app, QQmlApplicationEngine& engine, backend_instance& backend) -> bridge_instance;

} // namespace aba
