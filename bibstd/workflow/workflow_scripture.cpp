#include "bibstd/workflow/workflow_scripture.hpp"
#include "bibstd/bible/versification.hpp"
#include "bibstd/core/core_scripture_store.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/visit_helper.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

#include <memory>

namespace bibstd::workflow
{

///
///
workflow_scripture_settings::workflow_scripture_settings(std::shared_ptr<workflow_settings> workflow_settings)
  : framework::settings_base{std::move(workflow_settings)}
  , scripture_name{workflow_settings_->create_setting(
      "scripture.name",
      setting_value_t<decltype(scripture_name)>{},
      std::make_shared<framework::setting_validator_list<setting_value_t<decltype(scripture_name)>>>()
    )}
{
}

///
///
workflow_scripture::versification_wrapper::versification_wrapper(std::shared_ptr<bible::scripture> scripture)
  : data_{std::move(scripture)}
{
}

///
///
workflow_scripture::versification_wrapper::versification_wrapper(bible::scripture::versification_type&& versification)
  : data_{std::move(versification)}
{
}

///
///
auto workflow_scripture::versification_wrapper::get() const -> const bible::scripture::versification_type&
{
  return util::visit_lambdas(
    data_,
    [](const bible::scripture::versification_type& versification) -> const bible::scripture::versification_type&
    { return versification; },
    [](const std::shared_ptr<bible::scripture>& scripture) -> const bible::scripture::versification_type&
    { return scripture->versification(); }
  );
}

///
///
workflow_scripture::workflow_scripture(std::shared_ptr<workflow_settings> workflow_settings)
  : workflow_base{std::move(workflow_settings)}
  , core_scripture_store_(std::make_unique<core::core_scripture_store>())
{
  init();
}

///
///
workflow_scripture::~workflow_scripture() noexcept = default;

///
///
auto workflow_scripture::scripture(const scripture_params& params) -> scripture_result
{
  try
  {
    const auto lock = std::scoped_lock{mtx_};
    decltype(auto) scriptures = core_scripture_store_->scriptures();
    const auto scripture_name = params->scripture_name ? params->scripture_name : settings().scripture_name->value();
    auto result = scripture_result{return_failure};
    if(scripture_name)
    {
      if(const auto it = scriptures.find(*scripture_name); it != std::ranges::cend(scriptures))
      {
        result = scripture_result::value_type{.name = it->first, .scripture = it->second};
      }
      else
      {
        LOG_WARN("scripture name not found: \"{}\"", *scripture_name);
      }
    }
    else if(!scriptures.empty())
    {
      LOG_WARN("scripture name not set: using first scripture in store: \"{}\"", scriptures.begin()->first);
      result = scripture_result::value_type{.name = scriptures.begin()->first, .scripture = scriptures.begin()->second};
    }
    else
    {
      LOG_WARN("no scriptures loaded");
    }
    return result;
  }
  catch(...)
  {
    LOG_ERROR("exception occurred: {}", util::exception_report());
    return return_failure;
  }
}

///
///
auto workflow_scripture::passage(const passage_params& params) -> passage_result
{
  try
  {
    const auto lock = std::scoped_lock{mtx_};
    decltype(auto) scriptures = core_scripture_store_->scriptures();
    const auto scripture_name = params->scripture_name ? params->scripture_name : settings().scripture_name->value();
    auto result = passage_result{return_failure};
    if(scripture_name)
    {
      if(const auto it = scriptures.find(*scripture_name); it != std::ranges::cend(scriptures))
      {
        if(const auto passage_result = it->second->passage_html(params->reference))
        {
          result = passage_result::value_type{.passage = *passage_result};
        }
        else
        {
          LOG_WARN("passage not found: reference=\"{}\"", params->reference);
        }
      }
      else
      {
        LOG_WARN("scripture name not found: \"{}\"", *scripture_name);
      }
    }
    else
    {
      LOG_WARN("scripture name not set");
    }
    return result;
  }
  catch(...)
  {
    LOG_ERROR("exception occurred: {}", util::exception_report());
    return passage_result{return_failure};
  }
}

///
///
auto workflow_scripture::init() -> void
{
  const auto lock = std::scoped_lock{mtx_};
  decltype(auto) scriptures = core_scripture_store_->scriptures();
  const auto scripture_names = scriptures | std::views::keys | std::ranges::to<std::vector>();
  decltype(auto) scripture_name_validator =
    std::get<framework::setting_validator_list<std::optional<std::string>>::sptr_type>(settings().scripture_name->validator);
  std::ignore = scripture_name_validator->available(scripture_names);

  static constexpr auto has_kjv_versification = [](const auto& s)
  { return s.second->versification() == bible::versification_kjv; };

  if(const auto it = std::ranges::find_if(scriptures, has_kjv_versification); it != std::ranges::cend(scriptures))
  {
    settings().scripture_name->value(it->first);
  }
  else if(!scripture_names.empty())
  {
    settings().scripture_name->value(scripture_names.front());
  }
}

} // namespace bibstd::workflow
