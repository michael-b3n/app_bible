#pragma once

#include "bibstd/framework/task_queue.hpp"

#include <atomic>

namespace bibstd::framework
{

///
/// Run main loop.
///
class main_loop final
{
  // Variables
  inline static std::atomic_bool running_{false};
  inline static std::unique_ptr<task_queue> main_queue_{std::make_unique<task_queue>()};

public: // Modifiers
  ///
  /// Run main loop.
  ///
  static auto run() -> void;

  ///
  /// Exit main loop.
  ///
  static auto exit() noexcept -> void;

  ///
  /// Run task in main thread.
  /// \param task Task that shall be run in main thread.
  ///
  static auto queue_task(task_queue::task_type&& task) -> void;
};

} // namespace bibstd::framework
