#include "bibstd/workflow/workflow_scripture.hpp"
#include "bibstd/core/core_scripture_store.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"

#include <memory>

namespace bibstd::workflow
{

///
///
workflow_scripture_settings::workflow_scripture_settings()
  : scripture_name{workflow_settings_->create_setting(
      "scripture.name",
      setting_value_t<decltype(scripture_name)>{},
      std::make_shared<framework::setting_validator_list<setting_value_t<decltype(scripture_name)>>>()
    )}
{
}

///
///
workflow_scripture::workflow_scripture()
  : core_scripture_store_(std::make_unique<core::core_scripture_store>())
{
  init();
}

///
///
workflow_scripture::~workflow_scripture() noexcept = default;

///
///
auto workflow_scripture::get(const process_params& params) -> process_result
{
  try
  {
    const auto lock = std::scoped_lock{mtx_};
    const auto scripture_name = params->scripture_name ? params->scripture_name : settings->scripture_name->value();
    if(!scripture_name.has_value())
    {
      LOG_WARN("scripture name not set");
      return return_failure;
    }
    auto passage = core_scripture_store_->passage_html(*scripture_name, params->reference);
    return passage ? process_result{std::move(*passage)} : return_failure;
  }
  catch(const util::exception& e)
  {
    LOG_ERROR("exception occurred: {}", e);
    return return_failure;
  }
}

///
///
auto workflow_scripture::init() -> void
{
  const auto scripture_names = core_scripture_store_->scripture_names();
  decltype(auto) scripture_name_validator =
    std::get<framework::setting_validator_list<std::optional<std::string>>::sptr_type>(settings->scripture_name->validator);
  std::ignore = scripture_name_validator->available(scripture_names);
  if(!scripture_names.empty())
  {
    settings->scripture_name->value(scripture_names.front());
  }
}

} // namespace bibstd::workflow
