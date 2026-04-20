#pragma once

#include "src/construct_backend.hpp"

#include <bibqml/BridgeBibleRefOcr.hpp>
#include <bibqml/AbstractListModelPassage.hpp>

#include <QGuiApplication>
#include <QQmlApplicationEngine>

namespace aba
{

struct bridge_instance final
{
  // Variables
  std::unique_ptr<bibqml::BridgeBibleRefOcr> bridge_bible_ref_ocr;
  std::unique_ptr<bibqml::AbstractListModelPassage> abstract_list_model_passage;
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
/// \param engine The QQmlApplicationEngine instance
/// \param backend The backend instance
/// \return bridge instance
///
auto construct_bridge(QGuiApplication& app, QQmlApplicationEngine& engine, backend_instance& backend) -> bridge_instance;

} // namespace aba
