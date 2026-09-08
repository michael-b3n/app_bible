#pragma once

#include "qml/helpers/Translations.hpp"
#include "src/construct_backend.hpp"

#include <bibstd/framework/setting.hpp>
#include <bibstd/signal/synchronized_executor.hpp>
#include <bibstd/util/non_owning_ptr.hpp>

#include <string>

namespace aba
{

///
/// Instance holding the translations of the application.
/// This owns the QML translations singleton and keeps the language it displays its pretty
/// names in synchronized with the language setting. The translations themselves know nothing
/// about settings, and the backend knows nothing about translations.
///
class translations_instance final
{
  // Typedefs
  using language_setting_type = bibstd::util::non_owning_ptr<bibstd::framework::setting<std::string>>;

  // Variables
  const std::unique_ptr<qml::Translations> translations_;
  const language_setting_type language_setting_;
  bibstd::signal::synchronized_executor executor_;

public: // Structors
  ///
  /// Construct the translations instance.
  /// If no setting is provided, the pretty names stay in their default language.
  ///
  translations_instance(pretty_names names, language_setting_type language_setting);
  ~translations_instance() noexcept;

public: // Modifiers
  ///
  /// Disconnect all signal connections.
  /// This will stop the frontend backend communication.
  ///
  auto disconnect() -> void;
};

///
/// Initialize the translations of the application.
/// The pretty names are compiled into the application, the language they are displayed in is
/// stored in a setting that is created in the settings workflow of the backend.
/// \return translations instance, holding no pretty names if they could not be loaded
///
auto construct_translations(backend_instance& backend) -> translations_instance;

} // namespace aba
