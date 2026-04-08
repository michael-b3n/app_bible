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
auto tray::init(const icon_buffer icon, std::vector<entry_type>&& entries) -> util::shared_scope_guard
{
  static std::mutex _mtx;
  static util::shared_scope_guard::creator _guard_creator;
  static util::shared_scope_guard _thread_pool_guard;
  static std::condition_variable _cv_init;

  auto lock = std::unique_lock{_mtx};
  auto guard = _guard_creator.create(
    []()
    {
      tray_->exit();
      tray_.reset();
      worker_.reset();
      _thread_pool_guard.reset();
      _cv_init.notify_all();
    }
  );
  if(guard.is_initial_instance())
  {
    if(worker_)
    {
      _cv_init.wait(lock, []() { return !worker_; });
    }
    _thread_pool_guard = framework::thread_pool::init();
    worker_ = std::make_unique<framework::active_worker>();
    std::promise<void> promise{};
    auto future = promise.get_future();
    worker_->queue_task(
      [&]
      {
        static constexpr auto void_callback_wrapper = [](const auto& callback)
        {
          return [callback]()
          {
            auto f = callback;
            framework::thread_pool::queue_task(std::move(f));
          };
        };
        static constexpr auto toggle_callback_wrapper = [](const auto& callback)
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
  }
  return guard;
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
