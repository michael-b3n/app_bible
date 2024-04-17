#include "app_framework/single_worker.hpp"
#include "util/log.hpp"

#include <cassert>

namespace bibstd::app_framework
{

///
///
single_worker::scoped_guard::~scoped_guard() noexcept
{
  single_worker::shutdown();
}

///
///
auto single_worker::worker_id() -> std::optional<std::thread::id>
{
  return worker_id_.load();
}

///
///
auto single_worker::start() noexcept -> scoped_guard
{
  worker_ = std::jthread(
    [](std::stop_token stop_token) mutable
    {
      while(!stop_token.stop_requested())
      {
        try
        {
          while(!stop_token.stop_requested())
          {
            worker_queue_->empty() ? std::this_thread::sleep_for(std::chrono::milliseconds(10)) : worker_queue_->try_do_task();
          }
        }
        catch(const std::exception& e)
        {
          LOG_ERROR(single_worker::log_channel, "Worker queue error: {}", e.what());
        }
        catch(...)
        {
          LOG_ERROR(single_worker::log_channel, "Worker queue error: {}", "unknown exception");
        }
      }
    });
  worker_id_ = worker_.get_id();
  LOG_INFO(log_channel, "Init worker thread: id={}", "not_implemented" /*worker_.get_id()*/);
  assert(std::this_thread::get_id() != worker_.get_id());
  return scoped_guard{};
}

///
///
auto single_worker::shutdown() noexcept -> void
{
  assert(std::this_thread::get_id() != worker_.get_id());
  LOG_INFO(log_channel, "Stop worker thread: id={}", "not_implemented" /*worker_.get_id()*/);
  worker_id_ = std::nullopt;
  worker_.request_stop();
  worker_.join();
  worker_queue_->clear();
}

///
///
auto single_worker::queue_task(task_queue::task_type&& task) -> void
{
  worker_queue_->queue(std::forward<decltype(task)>(task));
}

///
///
auto run_in_worker_thread(task_queue::task_type&& task) -> void
{
  if(std::this_thread::get_id() == single_worker::worker_id())
  {
    task();
  }
  else
  {
    single_worker::queue_task(std::forward<decltype(task)>(task));
  }
}

} // namespace bibstd::app_framework
