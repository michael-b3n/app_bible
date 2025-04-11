#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <string_view>

namespace bibstd::app_framework
{

///
/// Task queue class.
///
class task_queue final
{
public: // Typedefs
  using task_type = std::move_only_function<void()>;

public: // Constructor
  ///
  /// Task queue constructor.
  ///
  task_queue() = default;

  ///
  /// Task queue destructor.
  ///
  ~task_queue() noexcept;

public: // Accessor
  ///
  /// Check if task queue is empty.
  /// \return true if empty, false otherwise
  ///
  auto empty() const -> bool;

  ///
  /// Get the count of all the tasks in queue.
  /// \return size of task queue
  ///
  auto size() const -> std::size_t;

public: // Modifiers
  ///
  /// Add task to queue.
  /// \param task that shall be added
  /// \warning The task must not destroy `this` on execution.
  ///
  auto queue(task_type&& task) -> void;

  ///
  /// Try to do one task in queue.
  /// If the queue lock can't be acquired immediately, no task is executed.
  ///
  auto try_do_task() -> void;

  ///
  /// Do one task in queue. If queue is empty, wait until a task is added.
  ///
  auto do_task_or_wait() -> void;

private: // Implementation
  auto do_task_impl(std::unique_lock<std::mutex>& queue_lock) -> void;

private: // Variables
  bool shutdown_{false};
  mutable std::mutex queue_mtx_;
  mutable std::mutex task_mtx_;
  std::condition_variable task_cv_;
  std::queue<task_type> task_queue_;
};

} // namespace bibstd::app_framework
