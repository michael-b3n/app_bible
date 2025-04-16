#pragma once

#include "app_framework/active_worker.hpp"
#include "app_framework/task_queue.hpp"
#include "util/non_owning_ptr.hpp"
#include "util/scoped_guard.hpp"
#include "util/uid.hpp"

#include <chrono>
#include <memory>
#include <thread>
#include <vector>

namespace bibstd::app_framework
{

///
/// Static thread_pool class.
///
class thread_pool final
{
public: // Constants
  inline static const auto max_thread_count = std::thread::hardware_concurrency();

public: // Typedefs
  using task_type = task_queue::task_type;
  using strand_id_type = util::uid<struct strand_id_tag>;

  ///
  /// Queue rule enum.
  ///
  enum class queue_rule
  {
    append,    // Append task to queue always
    overwrite, // Overwrite older tasks in queue if not running
    discard    // Discard task if older tasks are queued
  };

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
  ///
  static auto queue_task(task_type&& task) -> void;

  ///
  /// Queue task in thread pool.
  /// \param task Task that shall be run in thread_pool
  /// \param id Optional unique strand ID. If specified the task
  /// will run after the previous task with the same strand ID has finished.
  /// \param rule Queue rule \see queue_rule
  ///
  static auto queue_task(task_type&& task, strand_id_type id, queue_rule rule = queue_rule::append) -> void;

private: // Typedefs
  using task_id_type = util::uid<struct task_id_tag>;

  struct id_pair final
  {
    task_id_type task_id{};
    strand_id_type strand_id{};
  };

  struct pool_element final
  {
    active_worker worker{};
    std::vector<id_pair> ids{};
    std::chrono::system_clock::time_point last_use{std::chrono::system_clock::now()};
  };

  struct task_data final
  {
    task_type task{[] {}};
    task_id_type task_id{};
    strand_id_type strand_id{};
    queue_rule rule{queue_rule::append};
  };

private: // Implementation
  static auto queue_task_index(task_data&& data, std::size_t index) -> void;
  static auto queue_task_auto(task_data&& data) -> void;
  static auto create_task_wrapper(task_data&& data, util::non_owning_ptr<pool_element> element) -> task_type;
  static auto remove_abandoned_workers() -> void;

private: // Variables
  inline static std::mutex mtx_{};
  inline static std::vector<std::unique_ptr<pool_element>> pool_{};
};

} // namespace bibstd::app_framework
