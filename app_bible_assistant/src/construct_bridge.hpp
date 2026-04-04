#pragma once

#include "src/construct_backend.hpp"

#include <bibstd/util/scoped_guard.hpp>
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
  // QML bridge instances
  std::unique_ptr<bibqml::BridgeBibleRefOcr> bridge_bible_ref_ocr;
};

///
/// Initialize backend components.
/// \return backend instance
///
auto construct_bridge(QGuiApplication& app, QQmlApplicationEngine& engine, backend_instance& backend) -> bridge_instance;

} // namespace aba
