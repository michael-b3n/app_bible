#pragma once

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

public: // Constants
  static constexpr std::string_view log_channel = "task_queue";

public: // Constructor
  ///
  /// Task queue constructor.
  ///
  task_queue() = default;

  ///
  /// Task queue destructor.
  ///
  ~task_queue() noexcept = default;

public: // Modifiers
  ///
  /// Add task to queue.
  /// \param task that shall be added
  ///
  auto queue(task_type&& task) -> void;

  ///
  /// Try to do all tasks in queue.
  /// \return true, if all tasks were executed, false if not
  ///
  auto try_do_all() -> bool;

private: // Variables
  mutable std::mutex mtx_;
  std::queue<task_type> task_queue_;
};

} // namespace bibstd::app_framework
