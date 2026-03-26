#include "bibstd/workflow/workflow_bible_ref_lookup.hpp"
#include "bibstd/core/core_lookup_bibleserver.hpp"

namespace bibstd::workflow
{

///
///
// clang-format off
workflow_bible_ref_lookup_settings::workflow_bible_ref_lookup_settings()
  : translations{workflow_settings_->create_setting("lookup.translations", std::vector<bible::translation>{bible::translation::ngu, bible::translation::elb})}
// clang-format on
{
}

///
///
workflow_bible_ref_lookup::workflow_bible_ref_lookup()
  : core_lookup_bibleserver_{std::make_unique<core::core_lookup_bibleserver>()}
{
}

///
///
workflow_bible_ref_lookup::~workflow_bible_ref_lookup() noexcept = default;

///
///
auto workflow_bible_ref_lookup::lookup(const start_params& params) -> void
{
  try
  {
    framework::thread_pool::queue_task(
      [this, params]() mutable
      {
        const auto translations = settings->translations->value();
        try
        {
          std::ranges::for_each(
            params->references,
            [&](const auto& reference_range) { core_lookup_bibleserver_->open(reference_range, translations); }
          );
          emit<signal_id::ended>(result_params{params.process_id()});
        }
        catch(const util::exception& e)
        {
          LOG_ERROR("exception occurred: {}", e);
          emit<signal_id::ended>(result_params{params.process_id(), return_failure});
        }
      },
      strand_id_
    );
  }
  catch(const util::exception& e)
  {
    LOG_ERROR("exception occurred: {}", e);
    emit<signal_id::ended>(result_params{params.process_id(), std::unexpected{unexpected_result::failure}});
  }
}

} // namespace bibstd::workflow
