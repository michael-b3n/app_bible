#include "bibstd/workflow/workflow_bible_ref_lookup.hpp"
#include "bibstd/core/core_lookup_bibleserver.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"

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
  : thread_pool_guard_{framework::thread_pool::init()}
  , core_lookup_bibleserver_{std::make_unique<core::core_lookup_bibleserver>()}
{
}

///
///
workflow_bible_ref_lookup::~workflow_bible_ref_lookup() noexcept = default;

///
///
auto workflow_bible_ref_lookup::lookup(const params& params) -> void
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
          notify(&signals_type::ended, params.process_id());
        }
        catch(...)
        {
          LOG_ERROR("exception occurred: {}", util::exception_report());
          notify(&signals_type::ended, params.process_id());
        }
      },
      strand_id_
    );
  }
  catch(...)
  {
    LOG_ERROR("exception occurred: {}", util::exception_report());
    notify(&signals_type::ended, params.process_id());
  }
}

} // namespace bibstd::workflow
