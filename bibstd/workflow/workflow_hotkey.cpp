#include "bibstd/workflow/workflow_hotkey.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/system/hotkey.hpp"

#include <ranges>

namespace bibstd::workflow
{

///
///
workflow_hotkey::register_params::register_params(std::string&& path, callback_type&& callback)
  : path{std::move(path)}
  , callback{std::move(callback)}
{
}

///
///
workflow_hotkey::hotkey_params::hotkey_params(
  std::string&& path, const hotkey_type::key_modifier modifier, const hotkey_type::key key
)
  : path{std::move(path)}
  , modifier{modifier}
  , key{key}
{
}

///
///
workflow_hotkey::workflow_hotkey() = default;

///
///
workflow_hotkey::~workflow_hotkey() noexcept = default;

///
///
auto workflow_hotkey::available_callbacks() const -> std::vector<std::string>
{
  const auto lock = std::scoped_lock{mtx_};
  return callbacks_ | std::views::keys | std::ranges::to<std::vector<std::string>>();
}

///
///
auto workflow_hotkey::register_callback(const register_params& params) -> void
{
  const auto lock = std::scoped_lock{mtx_};
  callbacks_[params.path] = params.callback;
}

///
///
auto workflow_hotkey::assign_hotkey(const hotkey_params& params) -> bool
{
  const auto lock = std::scoped_lock{mtx_};
  const auto it = callbacks_.find(params.path);
  if(it == std::cend(callbacks_))
  {
    return false;
  }
  auto callback = it->second;
  system::hotkey::register_callback(
    params.key,
    params.modifier,
    [callback = std::move(callback)]() { framework::thread_pool::queue_task([callback]() { callback(); }); }
  );
  return true;
}

} // namespace bibstd::workflow
