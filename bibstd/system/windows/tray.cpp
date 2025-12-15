#include "bibstd/system/tray.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/visit_helper.hpp"

#include <algorithm>
#include <future>

namespace bibstd::system
{

///
///
auto tray::init(const icon_buffer icon, std::vector<entry_type>&& entries) -> util::scoped_guard
{
  worker_ = std::make_unique<framework::active_worker>();
  std::promise<void> promise{};
  auto future = promise.get_future();
  worker_->queue_task(
    [&]
    {
      const auto void_callback_wrapper = [](const auto& callback)
      {
        return [callback]()
        {
          auto f = callback;
          framework::thread_pool::queue_task(std::move(f));
        };
      };
      const auto toggle_callback_wrapper = [](const auto& callback)
      {
        return [callback](bool flag)
        {
          auto f = callback(flag);
          framework::thread_pool::queue_task(std::move(f));
        };
      };
      tray_ = std::make_unique<Tray::Tray>("system_tray_identifier", Tray::Icon(icon.buffer));
      std::ranges::for_each(
        entries,
        [&](const auto& entry)
        {
          util::visit_lambdas(
            entry,
            [&](const button& v) { tray_->addEntry(Tray::Button(v.text, void_callback_wrapper(v.callback))); },
            [&](const label& v) { tray_->addEntry(Tray::Label(v.text)); },
            [&](const separator& v) { tray_->addEntry(Tray::Separator()); },
            [&](const toggle& v) { tray_->addEntry(Tray::Toggle(v.text, v.state, toggle_callback_wrapper(v.callback))); }
          );
        }
      );
      promise.set_value();
      worker_->queue_task(get_message);
    }
  );
  future.get();
  return util::scoped_guard(
    []()
    {
      tray_->exit();
      tray_.reset();
      worker_.reset();
    }
  );
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
      LOG_ERROR("error getting message in file: {}:{}:{}", loc.file_name(), loc.line(), loc.column());
    }
    else
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    worker_->queue_task(get_message);
  }
}

} // namespace bibstd::system
