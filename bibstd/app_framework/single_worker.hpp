#pragma once

#include "app_framework/task_queue.hpp"
#include "util/non_copyable.hpp"

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>

namespace bibstd::app_framework
{

///
/// Static main worker class.
/// \warning Start called from parent thread.
///
class single_worker final
{
public: // Typedefs
  using task_type = std::function<void()>;

  ///
  /// Guard helper class for shutdown on destruction.
  ///
  struct guard : public util::non_copyable
  {
    ~guard() noexcept;
  };

public: // Constants
  static constexpr std::string_view log_channel = "single_worker";

public: // Accessor
  ///
  /// Get worker thread id.
  /// \return worker thread id if running, std::nullopt if not
  ///
  static auto worker_id() -> std::optional<std::thread::id>;

public: // Modifiers
  ///
  /// Start main worker thread.
  ///
  [[nodiscard]] static auto start() -> guard;

  ///
  /// Run task in worker thread.
  /// \param task Task that shall be run in worker thread.
  ///
  static auto queue_task(task_type&& task) -> void;

private: // Implementation
  ///
  /// Stop main worker thread and cleanup task queues.
  ///
  static auto shutdown() noexcept -> void;

private: // Variables
  inline static std::atomic<std::optional<std::thread::id>> worker_id_{std::nullopt};
  inline static std::unique_ptr<task_queue> worker_queue_ = std::make_unique<task_queue>();
  inline static std::jthread worker_ = std::jthread();
};

///
/// Move task to worker thread queue.
/// \param task Task that shall be executed in worker thread
///
auto run_in_worker_thread(single_worker::task_type&& task) -> void;

} // namespace bibstd::app_framework
