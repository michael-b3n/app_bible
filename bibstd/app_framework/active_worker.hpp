#pragma once

#include "app_framework/task_queue.hpp"

#include <memory>
#include <string_view>
#include <thread>

namespace bibstd::app_framework
{

///
/// Active worker class.
///
class active_worker final
{
public: // Constants
  static constexpr std::string_view log_channel = "active_worker";

public: // Structors
  active_worker();
  ~active_worker();

public: // Modifiers
  ///
  /// Queue task in active_worker thread.
  /// \param task Task that shall be run in active_worker thread
  ///
  auto queue_task(task_queue::task_type&& task) -> void;

  ///
  /// Run task in active_worker thread.
  /// \param task Task that shall be run in active_worker thread
  ///
  auto run_task(task_queue::task_type&& task) -> void;

private: // Implementation
  ///
  /// Stop main active_worker thread and cleanup task queues.
  ///
  auto shutdown() -> void;

private: // Variables
  std::unique_ptr<task_queue> worker_queue_;
  std::jthread worker_;
  const std::thread::id worker_id_;
};

} // namespace bibstd::app_framework
