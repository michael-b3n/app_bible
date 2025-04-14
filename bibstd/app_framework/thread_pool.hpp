#pragma once

#include "app_framework/active_worker.hpp"
#include "app_framework/task_queue.hpp"
#include "util/scoped_guard.hpp"
#include "util/uid.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>

namespace bibstd::app_framework
{

///
/// Static thread_pool class.
///
class thread_pool final
{
public: // Constants
  static constexpr std::uint16_t max_thread_count = 2;

public: // Typedefs
  using task_type = task_queue::task_type;
  using strand_id_type = util::uid<struct strand_id_tag>;
  using delay_type = std::chrono::nanoseconds;
  using time_point_type = std::chrono::time_point<std::chrono::system_clock>;

public: // Accessors
  ///
  /// Get a new unique strand ID.
  /// \return unique strand ID
  ///
  static auto strand_id() -> strand_id_type;

public: // Init
  ///
  /// Init thread pool.
  /// \return scoped guard to clean up the object on destruction
  ///
  static auto init() -> util::scoped_guard;

public: // Modifiers
  ///
  /// Queue task in thread pool.
  /// \param task Task that shall be run in thread_pool
  /// \param id Optional unique strand ID. If specified the task
  /// will run after the previous task with the same strand ID has finished.
  ///
  static auto queue_task(task_type&& task, std::optional<strand_id_type> id = std::nullopt) -> void;

private: // Typedefs
  struct delayed_task final
  {
    task_type task_;
    std::optional<strand_id_type> strand_id_;
    time_point_type run_time_;
  };

private: // Implementation
  static auto queue_task_impl(task_type&& task, std::optional<strand_id_type> id) -> void;
  static auto queue_task_index(task_type&& task, strand_id_type id, std::size_t index) -> void;
  static auto queue_task_auto(task_type&& task, strand_id_type id) -> void;
  static auto create_task_wrapper(task_type&& task, std::size_t worker_index) -> task_type;

private: // Variables
  inline static std::mutex mtx_{};
  inline static std::array<std::unique_ptr<active_worker>, max_thread_count> pool_{};
  inline static std::array<std::deque<strand_id_type>, max_thread_count> pool_ids_{};
  inline static std::vector<delayed_task> delayed_tasks_{};
};

} // namespace bibstd::app_framework
