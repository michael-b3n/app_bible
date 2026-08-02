#include "src/construct_translations.hpp"

#include <bibstd/framework/setting_validator.hpp>
#include <bibstd/util/exception.hpp>
#include <bibstd/util/incbin.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <QMetaObject>

#include <memory>
#include <utility>

INC_RESOURCE(pretty_names, "res/pretty_names.csv");
const auto pretty_names_view = bibstd::util::incbin::to_span<std::byte>(res_pretty_names_data, res_pretty_names_size);

namespace aba
{

///
///
translations_instance::translations_instance(pretty_names names, const language_setting_type language_setting)
  : translations_{std::make_unique<qml::Translations>(std::move(names))}
  , language_setting_{language_setting}
{
  if(!language_setting_)
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
translations_instance::~translations_instance() noexcept = default;

///
///
auto translations_instance::disconnect() -> void
{
  executor_.disconnect();
}

///
///
auto construct_translations(backend_instance& backend) -> translations_instance
{
  try
  {
    auto names = pretty_names{pretty_names_view};
    const auto language_setting = backend.workflow_settings->create_setting(
      "ui.language",
      std::string{names.languages().front()},
      std::make_shared<bibstd::framework::setting_validator_list<std::string>>(names.languages())
    );
    return translations_instance{std::move(names), language_setting};
  }
  catch(...)
  {
    LOG_ERROR("construct translations failed: {}", bibstd::util::exception_report());
    return translations_instance{pretty_names{}, nullptr};
  }
}

} // namespace aba
