#include "app_framework/task_queue.hpp"
#include "util/log.hpp"

namespace bibstd::app_framework
{

///
///
task_queue::~task_queue() noexcept
{
  shutdown();
}

///
///
auto task_queue::empty() const -> bool
{
  const auto lock = std::lock_guard(queue_mtx_);
  return task_queue_.empty();
}

///
///
auto task_queue::size() const -> std::size_t
{
  const auto lock = std::lock_guard(queue_mtx_);
  return task_queue_.size();
}

///
///
auto task_queue::queue(task_type&& task) -> void
{
  const auto lock = std::lock_guard(queue_mtx_);
  task_queue_.emplace(std::move(task));
}

///
///
auto task_queue::try_do_tasks() noexcept -> void
{
  try
  {
    while(!empty())
    {
      std::unique_lock<std::mutex> queue_lock(queue_mtx_, std::defer_lock);
      const auto queue_locked = queue_lock.try_lock();
      if(queue_locked && !task_queue_.empty())
      {
        decltype(auto) task = std::move(task_queue_.front());
        task_queue_.pop();
        queue_lock.unlock();
        const auto task_lock = std::lock_guard(task_mtx_);
        task();
      }
    }
  }
  catch(const std::exception& e)
  {
    LOG_ERROR(task_queue::log_channel, "Task queue error: {}", e.what());
  }
}

///
///
auto task_queue::shutdown() noexcept -> void
{
  const auto queue_lock = std::lock_guard(queue_mtx_);
  {
    std::queue<task_type> empty_queue;
    std::swap(task_queue_, empty_queue);
  }
  const auto task_lock = std::lock_guard(task_mtx_);
}

} // namespace bibstd::app_framework
