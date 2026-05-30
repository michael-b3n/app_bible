#include "bibstd/workflow/workflow_hotkey.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/system/hotkey.hpp"

#include <ranges>

namespace bibstd::workflow
{

///
///
workflow_hotkey::assign_params_t::assign_params_t(
  std::string path, const hotkey_type::key_modifier modifier, const hotkey_type::key key
)
  : path{std::move(path)}
  , modifier{modifier}
  , key{key}
{
}

///
///
workflow_hotkey::workflow_hotkey()
  : thread_pool_guard_{framework::thread_pool::init()}
  , hotkey_guard_{system::hotkey::init()}
{
}

///
///
workflow_hotkey::~workflow_hotkey() noexcept = default;

///
///
auto workflow_hotkey::available_callbacks() const -> std::vector<std::string>
{
  const auto lock = std::scoped_lock{mtx_};
  return shared_sigs_ | std::views::keys | std::ranges::to<std::vector<std::string>>();
}

///
///
auto workflow_hotkey::register_callback(const path_type& path) -> shared_sig_type
{
  const auto lock = std::scoped_lock{mtx_};
  if(shared_sigs_.contains(path))
  {
    return shared_sigs_.at(path);
  }
  auto shared_sig = std::make_shared<signal::signal_type<void()>>();
  shared_sigs_[path] = shared_sig;
  return shared_sig;
}

///
///
auto workflow_hotkey::assign_hotkey(const assign_params& params) -> bool
{
  const auto lock = std::scoped_lock{mtx_};
  const auto it = shared_sigs_.find(params->path);
  if(it == std::cend(shared_sigs_))
  {
    return false;
  }
  auto shared_sig = it->second;
  system::hotkey::unregister_callback(params->key, params->modifier);
  system::hotkey::register_callback(
    params->key,
    params->modifier,
    [sig = std::weak_ptr{shared_sig}]()
    {
      framework::thread_pool::queue_task(
        [sig]()
        {
          if(auto s = sig.lock())
          {
            (*s)();
          }
        }
      );
    }
  );
  return true;
}

} // namespace bibstd::workflow
