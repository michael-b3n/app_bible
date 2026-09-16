#pragma once

#include "qml/helpers/Translations.hpp"
#include "src/construct_backend.hpp"

#include <bibstd/framework/setting.hpp>
#include <bibstd/signal/synchronized_executor.hpp>
#include <bibstd/util/non_owning_ptr.hpp>

#include <optional>
#include <string>
#include <string_view>

namespace aba
{

///
/// Read the pretty names compiled into the application.
/// \throws util::exception if they cannot be parsed
/// \return table of pretty names
///
[[nodiscard]] auto compiled_pretty_names() -> app_pretty_names;

///
/// Read the language the pretty names are displayed in from the settings file. The file is only
/// read, which lets an instance owning no settings display the language of the one that does.
/// \return language, std::nullopt if the file holds none
///
[[nodiscard]] auto read_language_setting() -> std::optional<std::string>;

///
/// Instance holding the translations of the application.
/// This owns the QML translations singleton and keeps the language it displays its pretty
/// names in synchronized with the language setting. The translations themselves know nothing
/// about settings, and the backend knows nothing about translations.
///
class translations_instance final
{
  // Variables
  const std::unique_ptr<qml::Translations> translations_;
  const bibstd::util::non_owning_ptr<bibstd::framework::setting<std::string>> language_setting_;
  bibstd::signal::synchronized_executor executor_;

public: // Typedefs
  using language_setting_type = decltype(language_setting_);

public: // Constants
  static constexpr auto language_setting_path = std::string_view{"ui.language"};

public: // Structors
  ///
  /// Construct the translations instance.
  /// If no setting is provided, the pretty names stay in their default language.
  ///
  translations_instance(app_pretty_names names, language_setting_type language_setting);

  ///
  /// Construct the translations instance without a setting to follow.
  /// The pretty names are displayed in the given language and never change afterwards.
  ///
  translations_instance(app_pretty_names names, const std::optional<std::string>& language);

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

///
/// Initialize the translations of an application that owns no settings.
/// The pretty names are displayed in \p language, in the default language if not set.
/// \return translations instance, holding no pretty names if they could not be loaded
///
auto construct_translations(const std::optional<std::string>& language) -> translations_instance;

} // namespace aba
