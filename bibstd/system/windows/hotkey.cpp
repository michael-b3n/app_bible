#include "system/hotkey.hpp"
#include "system/windows/hotkey_impl.hpp"

namespace bibstd::system
{

///
///
auto hotkey::init() -> util::scoped_guard
{
  return hotkey_impl::init();
}

///
///
auto hotkey::register_callback(const key key, const key_modifier mod, std::function<void()>&& callback) -> void
{
  hotkey_impl::register_callback(key, mod, std::move(callback));
}

///
///
auto hotkey::unregister_callback(const key key, const key_modifier mod) -> void
{
  hotkey_impl::unregister_callback(key, mod);
}

} // namespace bibstd::system
