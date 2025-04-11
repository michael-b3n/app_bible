#pragma once

#include "app_framework/task_queue.hpp"

#include <atomic>
#include <string_view>
#include <thread>

namespace bibstd::app_framework
{

///
/// Run main loop.
///
class main_loop final
{
public: // Constants
  static constexpr std::string_view log_channel = "main_loop";

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

private: // Variables
  inline static std::atomic_bool running_{false};
  inline static std::unique_ptr<task_queue> main_queue_{std::make_unique<task_queue>()};
};

} // namespace bibstd::app_framework
