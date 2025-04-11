#include "app_framework/active_worker.hpp"
#include "util/exception.hpp"

namespace bibstd::app_framework
{

///
///
active_worker::active_worker()
  : worker_queue_{std::make_unique<task_queue>()}
  , worker_{std::jthread(
      [this](std::stop_token stop_token) mutable
      {
        while(!stop_token.stop_requested())
        {
          try
          {
            while(!stop_token.stop_requested())
            {
              worker_queue_->do_task_or_wait();
            }
          }
          catch(const std::exception& e)
          {
            LOG_ERROR(log_channel, "Worker queue error: {}", e.what());
          }
          catch(...)
          {
            LOG_ERROR(log_channel, "Worker queue error: {}", "unknown exception");
          }
        }
      }
    )}
  , worker_id_{worker_.get_id()}
{
  LOG_INFO(log_channel, "Start active_worker thread: id={}", worker_.get_id());
}

///
///
active_worker::~active_worker()
{
  shutdown();
}

///
///
auto active_worker::queue_task(task_queue::task_type&& task) -> void
{
  worker_queue_->queue(std::forward<decltype(task)>(task));
}

///
///
auto active_worker::run_task(task_queue::task_type&& task) -> void
{
  if(std::this_thread::get_id() == worker_id_)
  {
    task();
  }
  else
  {
    queue_task(std::forward<decltype(task)>(task));
  }
}

///
///
auto active_worker::shutdown() -> void
{
  LOG_INFO(log_channel, "Stop active_worker thread: id={}", worker_.get_id());
  worker_.request_stop();
  worker_queue_.reset();
  worker_.join();
}

} // namespace bibstd::app_framework
