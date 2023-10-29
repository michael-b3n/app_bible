#include "app_framework/task_queue.hpp"
#include "util/log.hpp"

namespace bibstd::app_framework
{
namespace detail
{

///
/// Calls task wrapped into a try catch block.
/// \param task that shall be called
///
auto call_task(task_queue::task_type& task) -> void
{
  try
  {
    task();
  }
  catch(const std::exception& e)
  {
    LOG_ERROR(task_queue::log_channel, "Task queue error: {}", e.what());
  }
}

} // namespace detail

///
///
auto task_queue::queue(task_type&& task) -> void
{
  const auto lock = std::lock_guard(mtx_);
  task_queue_.emplace(std::move(task));
}

///
///
auto task_queue::try_do_all() -> bool
{
  std::unique_lock<std::mutex> lock(mtx_, std::defer_lock);
  const auto locked = lock.try_lock();
  if(locked)
  {
    while(!task_queue_.empty())
    {
      decltype(auto) task = task_queue_.front();
      detail::call_task(task);
      task_queue_.pop();
    }
  }
  return locked;
}

} // namespace bibstd::app_framework
