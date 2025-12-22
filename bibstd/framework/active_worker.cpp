#include "bibstd/framework/active_worker.hpp"
#include "bibstd/util/exception.hpp"

namespace bibstd::framework
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
          catch(const util::exception& e)
          {
            LOG_ERROR("worker queue error: {}", e.what());
          }
          catch(...)
          {
            LOG_ERROR("worker queue error: {}", "unknown exception");
          }
        }
      }
    )}
  , worker_id_{worker_.get_id()}
{
  LOG_INFO("worker start thread: id={}", worker_.get_id());
}

///
///
active_worker::~active_worker() noexcept
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
auto active_worker::shutdown() -> void
{
  LOG_INFO("worker stop thread: id={}", worker_.get_id());
  worker_.request_stop();
  worker_queue_.reset();
  worker_.join();
}

} // namespace bibstd::framework
