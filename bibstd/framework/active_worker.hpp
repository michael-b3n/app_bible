#pragma once

#include "bibstd/framework/task_queue.hpp"

#include <memory>
#include <thread>

namespace bibstd::framework
{

///
/// Active worker class.
///
class active_worker final
{
  // Variables
  std::unique_ptr<task_queue> worker_queue_;
  std::jthread worker_;
  const std::thread::id worker_id_;

public: // Structors
  active_worker();
  ~active_worker() noexcept;

public: // Modifiers
  ///
  /// Queue task in active_worker thread.
  /// \param task Task that shall be run in active_worker thread
  ///
  auto queue_task(task_queue::task_type&& task) -> void;

private: // Implementation
  ///
  /// Stop main active_worker thread and cleanup task queues.
  ///
  auto shutdown() -> void;
};

} // namespace bibstd::framework
