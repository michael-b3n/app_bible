#include "app_framework/main_loop.hpp"
#include "util/log.hpp"

#include <cassert>

namespace bibstd::app_framework
{

///
///
auto main_loop::run(task_queue::task_type&& iteration_task) -> void
{
  auto task = std::move(iteration_task);
  assert(!running_);
  running_ = true;
  while(running_)
  {
    try
    {
      while(running_)
      {
        main_queue_->try_do_task();
        task();
      }
    }
    catch(const std::exception& e)
    {
      LOG_ERROR(log_channel, "Main queue error: {}", e.what());
    }
    catch(...)
    {
      LOG_ERROR(log_channel, "Main queue error: {}", "unknown exception");
    }
  }
}

///
///
auto main_loop::exit() noexcept -> void
{
  running_ = false;
  main_queue_->clear();
}

///
///
auto main_loop::queue_task(task_queue::task_type&& task) -> void
{
  main_queue_->queue(std::forward<decltype(task)>(task));
}

///
///
auto run_in_main_thread(task_queue::task_type&& task) -> void
{
  main_loop::queue_task(std::forward<decltype(task)>(task));
}

} // namespace bibstd::app_framework
