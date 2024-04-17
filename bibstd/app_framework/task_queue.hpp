#pragma once

#include <atomic>
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
  using task_type = std::function<void()>;

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
  ///
  auto queue(task_type&& task) -> void;

  ///
  /// Try to do one task in queue.
  /// If the queue lock can't be acquired immediately, no task is executed.
  ///
  auto try_do_task() -> void;

  ///
  /// Clear queue.
  ///
  auto clear() -> void;

private: // Variables
  mutable std::mutex queue_mtx_;
  mutable std::mutex task_mtx_;
  std::queue<task_type> task_queue_;
};

} // namespace bibstd::app_framework
