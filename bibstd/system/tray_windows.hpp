#pragma once

#include "app_framework/active_worker.hpp"
#include "system/tray_base.hpp"
#include "util/log.hpp"
#include "util/string.hpp"
#include "util/visit_helper.hpp"

#include <tray.hpp>

#include <algorithm>
#include <cassert>
#include <future>
#include <memory>
#include <ranges>
#include <source_location>

namespace bibstd::system
{

///
/// Key input class for windows implementation.
///
class tray final : public tray_base
{
public: // Typedefs
#include BOOST_PP_UPDATE_COUNTER()
  using active_worker_type = app_framework::active_worker<BOOST_PP_COUNTER>;

public: // Static modifiers
  static inline auto init(std::filesystem::path&& icon_path, std::vector<entry_type>&& entries) -> util::scoped_guard;

private: // Static helpers
  static auto get_message() -> void;

private:
  inline static std::unique_ptr<Tray::Tray> tray_{nullptr};
  inline static std::map<int, std::function<void()>> callback_map_{};
};

///
///
inline auto tray::init(std::filesystem::path&& icon_path, std::vector<entry_type>&& entries) -> util::scoped_guard
{
  decltype(auto) worker_guard = active_worker_type::start(
    [icon_path, entries]()
    {
      const auto void_callback_wrapper = [](const auto& callback)
      {
        return [callback]()
        {
          auto f = callback;
          app_framework::active_worker_main::run_task(std::move(f));
        };
      };
      const auto toggle_callback_wrapper = [](const auto& callback)
      {
        return [callback](bool flag)
        {
          auto f = callback(flag);
          app_framework::active_worker_main::run_task(std::move(f));
        };
      };
      tray_ = std::make_unique<Tray::Tray>("system_tray_identifier", util::to_string(icon_path.generic_u8string()));
      std::ranges::for_each(
        entries,
        [&](const auto& entry)
        {
          util::visit_lambdas(
            entry,
            [&](const button& v) { tray_->addEntry(Tray::Button(v.text, void_callback_wrapper(v.callback))); },
            [&](const label& v) { tray_->addEntry(Tray::Label(v.text)); },
            [&](const separator& v) { tray_->addEntry(Tray::Separator()); },
            [&](const toggle& v) { tray_->addEntry(Tray::Toggle(v.text, v.state, toggle_callback_wrapper(v.callback))); },
            [&](const submenu& submenu)
            {
              const auto submenu_sptr = tray_->addEntry(Tray::Submenu(submenu.text));
              std::ranges::for_each(
                submenu.entries,
                [&](const auto& submenu_entry)
                {
                  util::visit_lambdas(
                    submenu_entry,
                    [&](const button& v) { submenu_sptr->addEntry(Tray::Button(v.text, void_callback_wrapper(v.callback))); },
                    [&](const label& v) { submenu_sptr->addEntry(Tray::Label(v.text)); },
                    [&](const separator& v) { submenu_sptr->addEntry(Tray::Separator()); },
                    [&](const toggle& v) { submenu_sptr->addEntry(Tray::Toggle(v.text, v.state, toggle_callback_wrapper(v.callback))); });
                });
            });
        });
      active_worker_type::queue_task(get_message);
    });
  return util::scoped_guard(
    [guard = std::move(worker_guard)]()
    {
      tray_->exit();
      tray_.reset();
    });
}

///
///
auto tray::get_message() -> void
{
  static MSG msg;
  if(const auto ret = GetMessage(&msg, nullptr, 0, 0); ret != 0)
  {
    if(ret == -1)
    {
      const std::source_location loc = std::source_location::current();
      LOG_ERROR(log_channel, "Error getting message in file: {}:{}:{}", loc.file_name(), loc.line(), loc.column());
    }
    else
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    active_worker_type::queue_task(get_message);
  }
}

} // namespace bibstd::system
