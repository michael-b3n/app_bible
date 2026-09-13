#include "bibstd/system/tray.hpp"
#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/visit_helper.hpp"

#include <algorithm>
#include <future>

namespace bibstd::system
{
namespace
{

///
/// Convert UTF-8 text to the ANSI code page, the tray menu is built with the ANSI API.
/// Characters the code page cannot represent are replaced.
///
[[nodiscard]] auto to_ansi(const std::string& text) -> std::string
{
  const auto wide_count = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
  if(wide_count <= 0)
  {
    return text;
  }
  auto wide = std::wstring(static_cast<std::size_t>(wide_count), wchar_t{});
  MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), wide_count);
  const auto ansi_count = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if(ansi_count <= 0)
  {
    return text;
  }
  auto ansi = std::string(static_cast<std::size_t>(ansi_count), char{});
  WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, ansi.data(), ansi_count, nullptr, nullptr);
  ansi.pop_back(); // terminating null
  return ansi;
}

} // namespace

///
///
auto tray::init(const icon_buffer icon, std::vector<entry_type>&& entries) -> util::shared_scope_guard
{
  static util::shared_scope_guard::creator guard_creator;
  static util::shared_scope_guard thread_pool_guard;
  static std::condition_variable cv_init;

  auto lock = std::unique_lock{mtx_};
  auto guard = guard_creator.create(
    []()
    {
      const auto lock = std::scoped_lock{mtx_};
      tray_->exit();
      // The tray thread may still run a task using the tray, so it is joined first.
      worker_.reset();
      tray_.reset();
      thread_pool_guard.reset();
      cv_init.notify_all();
    }
  );
  if(guard.is_initial_instance())
  {
    if(worker_)
    {
      cv_init.wait(lock, []() { return !worker_; });
    }
    thread_pool_guard = framework::thread_pool::init();
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
        thread_id_ = GetCurrentThreadId();
        tray_ = std::make_unique<Tray::Tray>("system_tray_identifier", Tray::Icon(icon.buffer));
        std::ranges::for_each(
          entries,
          [&](const auto& entry)
          {
            util::visit_lambdas(
              entry,
              [&](const button& v) { tray_->addEntry(Tray::Button(to_ansi(v.text), void_callback_wrapper(v.callback))); },
              [&](const label& v) { tray_->addEntry(Tray::Label(to_ansi(v.text))); },
              [&]([[maybe_unused]] const separator& /*v*/) { tray_->addEntry(Tray::Separator()); },
              [&](const toggle& v)
              { tray_->addEntry(Tray::Toggle(to_ansi(v.text), v.state, toggle_callback_wrapper(v.callback))); }
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
auto tray::set_text(const std::size_t index, std::string text) -> void
{
  const auto lock = std::scoped_lock{mtx_};
  if(!worker_)
  {
    return;
  }
  // The menu belongs to the tray thread, so it is rebuilt there.
  worker_->queue_task(
    [index, text = to_ansi(text)]() mutable
    {
      if(!tray_)
      {
        return;
      }
      const auto entries = tray_->getEntries();
      if(index < entries.size())
      {
        entries[index]->setText(std::move(text));
      }
    }
  );
  // The tray thread waits for a message, the task only runs once it got one.
  PostThreadMessage(thread_id_, WM_NULL, 0, 0);
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
