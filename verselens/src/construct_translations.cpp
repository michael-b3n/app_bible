#include "src/construct_translations.hpp"
#include "res/version.hpp"

#include <bibstd/framework/setting_validator.hpp>
#include <bibstd/util/exception.hpp>
#include <bibstd/util/incbin.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <QMetaObject>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <format>
#include <memory>
#include <utility>

INC_RESOURCE(pretty_names, "res/pretty_names.csv");
const auto pretty_names_view = bibstd::util::incbin::to_span<std::byte>(res_pretty_names_data, res_pretty_names_size);

namespace verselens
{

///
///
translations_instance::translations_instance(app_pretty_names names, const language_setting_type language_setting)
  : translations_{std::make_unique<qml::Translations>(std::move(names))}
  , language_setting_{language_setting}
{
  if(language_setting_ == nullptr)
  {
    // Without a language setting the translations stay in their default language.
    return;
  }
  translations_->setLanguage(QString::fromStdString(language_setting_->value()));
  language_setting_->connect_queued(
    &bibstd::framework::setting_signals::value_changed,
    [this]()
    {
      QMetaObject::invokeMethod(
        translations_.get(),
        [this]() { translations_->setLanguage(QString::fromStdString(language_setting_->value())); },
        Qt::QueuedConnection
      );
    },
    executor_
  );
}

///
///
translations_instance::translations_instance(app_pretty_names names, const std::optional<std::string>& language)
  : translations_{std::make_unique<qml::Translations>(std::move(names))}
  , language_setting_{nullptr}
{
  if(language.has_value())
  {
    translations_->setLanguage(QString::fromStdString(*language));
  }
}

///
///
translations_instance::~translations_instance() noexcept = default;

///
///
auto translations_instance::disconnect() -> void
{
  executor_.disconnect();
}

///
///
auto compiled_pretty_names() -> app_pretty_names
{
  return app_pretty_names{pretty_names_view};
}

///
///
auto read_language_setting() -> std::optional<std::string>
{
  try
  {
    auto tree = boost::property_tree::ptree{};
    boost::property_tree::read_xml(
      bibstd::workflow::workflow_settings::settings_file_path(version::data_folder_name).generic_string(), tree
    );
    const auto language = tree.get_optional<std::string>(std::format(
      "{}.{}", bibstd::workflow::workflow_settings::settings_root_name, translations_instance::language_setting_path
    ));
    return language ? std::optional{*language} : std::nullopt;
  }
  catch(...)
  {
    return std::nullopt;
  }
}

///
///
auto construct_translations(backend_instance& backend) -> translations_instance
{
  try
  {
    auto names = compiled_pretty_names();
    auto* const language_setting = backend.workflow_settings->create_setting(
      std::string{translations_instance::language_setting_path},
      std::string{names.languages().front()},
      std::make_shared<bibstd::framework::setting_validator_list<std::string>>(names.languages())
    );
    return translations_instance{std::move(names), language_setting};
  }
  catch(...)
  {
    LOG_ERROR("construct translations failed: {}", bibstd::util::exception_report());
    return translations_instance{app_pretty_names{}, std::nullopt};
  }
}

///
///
auto construct_translations(const std::optional<std::string>& language) -> translations_instance
{
  try
  {
    return translations_instance{compiled_pretty_names(), language};
  }
  catch(...)
  {
    LOG_ERROR("construct translations failed: {}", bibstd::util::exception_report());
    return translations_instance{app_pretty_names{}, std::nullopt};
  }
}

} // namespace verselens
