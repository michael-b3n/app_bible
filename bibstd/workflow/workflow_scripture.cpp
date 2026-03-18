#include "bibstd/workflow/workflow_scripture.hpp"
#include "bibstd/core/core_scripture_store.hpp"
#include <memory>

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_scripture_settings::workflow_scripture_settings()
  : scripture_name{workflow_settings_->create_setting("scripture.name", setting_value_t<decltype(scripture_name)>{}, std::make_shared<framework::setting_validator_list<setting_value_t<decltype(scripture_name)>>>())}
// clang-format on
{
}

///
///
workflow_scripture::workflow_scripture()
  : core_scripture_store_(std::make_unique<core::core_scripture_store>())
{
  decltype(auto) scripture_name_validator =
    std::get<framework::setting_validator_list<std::optional<std::string>>::sptr_type>(settings->scripture_name->validator);
  std::ignore = scripture_name_validator->available(
    core_scripture_store_->available_scriptures() | std::views::transform(&core::core_scripture_store::scripture_info::name)
  );
}

///
///
workflow_scripture::~workflow_scripture() noexcept = default;

///
///
auto workflow_scripture::start(const start_params& params) -> std::stop_source
{
  const std::stop_source stop_source;
  // TODO: Implement workflow logic
  emit<signal_id::ended>(result_type{params.process_id(), std::unexpected{unexpected_result::failure}});
  return stop_source;
}

} // namespace bibstd::workflow
