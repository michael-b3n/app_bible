#include "app_framework/main_loop.hpp"
#include "util/log.hpp"

#include <cassert>

namespace bibstd::app_framework
{

///
///
auto main_loop::run() -> void
{
  assert(!running_);
  running_ = true;
  while(running_)
  {
    try
    {
      while(running_)
      {
        main_queue_->do_task_or_wait();
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
  main_queue_->queue([]() {}); // queue dummy task and trigger notify
}

///
///
auto main_loop::queue_task(task_queue::task_type&& task) -> void
{
  if(running_)
  {
    main_queue_->queue(std::forward<decltype(task)>(task));
  }
}

} // namespace bibstd::app_framework
