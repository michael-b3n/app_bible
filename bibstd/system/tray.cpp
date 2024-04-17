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
tray::tray(std::string&& identifier, std::filesystem::path&& icon_path)
  : tray_{std::make_unique<Tray::Tray>(std::forward<decltype(identifier)>(identifier), util::to_string(icon_path.u8string()))}
{
}

///
///
tray::~tray() noexcept
{
  try
  {
    exit();
  }
  catch(const std::exception& e)
  {
    LOG_ERROR(log_channel, "Unknown exception on tray exit: {}", e.what());
  }
}

///
///
auto tray::main_iteration_do() -> void
{
  tray_->main_iteration_do();
}

///
///
auto tray::exit() -> void
{
  tray_->exit();
}

///
///
auto tray::update() -> void
{
  tray_->update();
}

///
///
auto tray::add_button(button&& button) -> void
{
  tray_->addEntry(Tray::Button(button.text, button.callback));
}

///
///
auto tray::add_label(label&& label) -> void
{
  tray_->addEntry(Tray::Label(label.text));
}

///
///
auto tray::add_separator() -> void
{
  tray_->addEntry(Tray::Separator());
}

///
///
auto tray::add_toggle(toggle&& toggle) -> void
{
  tray_->addEntry(Tray::Toggle(toggle.text, toggle.state, toggle.callback));
}

///
///
auto tray::add_submenu(submenu&& submenu) -> void
{
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
