#include "system/tray.hpp"
#include "util/log.hpp"
#include "util/string.hpp"
#include "util/visit_helper.hpp"

#include <algorithm>
#include <tray.hpp>

namespace bibstd::system
{

///
///
tray::tray(const std::string identifier, std::filesystem::path&& icon_path)
  : identifier_{identifier}
  , tray_{std::make_unique<Tray::Tray>(identifier, util::to_string(icon_path.u8string()))}
{
}

///
///
tray::~tray() noexcept
{
  try
  {
    reset();
  }
  catch(const std::exception& e)
  {
    LOG_ERROR(log_channel, "Unknown exception on tray exit: {}", e.what());
  }
}

///
///
auto tray::reset() -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(tray_)
  {
    tray_->exit();
    tray_.reset();
  }
}

///
///
auto tray::update() -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(!tray_)
  {
    LOG_ERROR(log_channel, "Failed to update invalidated tray={}", identifier_);
    return;
  }
  tray_->update();
}

///
///
auto tray::add_exit_button(button&& button) -> void
{
  add_button(tray::button{
    button.text,
    [this, callback = std::move(button.callback)]()
    {
      callback();
      reset();
    }});
}

///
///
auto tray::add_button(button&& button) -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(!tray_)
  {
    LOG_ERROR(log_channel, "Failed to add button to invalidated tray={}", identifier_);
    return;
  }
  tray_->addEntry(Tray::Button(button.text, button.callback));
}

///
///
auto tray::add_label(label&& label) -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(!tray_)
  {
    LOG_ERROR(log_channel, "Failed to add label to invalidated tray={}", identifier_);
    return;
  }
  tray_->addEntry(Tray::Label(label.text));
}

///
///
auto tray::add_separator() -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(!tray_)
  {
    LOG_ERROR(log_channel, "Failed to add separator to invalidated tray={}", identifier_);
    return;
  }
  tray_->addEntry(Tray::Separator());
}

///
///
auto tray::add_toggle(toggle&& toggle) -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(!tray_)
  {
    LOG_ERROR(log_channel, "Failed to add toggle to invalidated tray={}", identifier_);
    return;
  }
  tray_->addEntry(Tray::Toggle(toggle.text, toggle.state, toggle.callback));
}

///
///
auto tray::add_submenu(submenu&& submenu) -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(!tray_)
  {
    LOG_ERROR(log_channel, "Failed to add submenu to invalidated tray={}", identifier_);
    return;
  }
  const auto entry = tray_->addEntry(Tray::Submenu(submenu.text));
  std::ranges::for_each(
    submenu.entries,
    [&](const auto& e)
    {
      util::visit_lambdas(
        e,
        [&](const button& v) { entry->addEntry(Tray::Button(v.text, v.callback)); },
        [&](const label& v) { entry->addEntry(Tray::Label(v.text)); },
        [&](const separator& v) { entry->addEntry(Tray::Separator()); },
        [&](const toggle& v) { entry->addEntry(Tray::Toggle(v.text, v.state, v.callback)); });
    });
}

} // namespace bibstd::system
