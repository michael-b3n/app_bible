#pragma once

#include "app_framework/task_queue.hpp"
#include "util/scoped_guard.hpp"

#include <boost/preprocessor/slot/counter.hpp>

#include <atomic>
#include <cassert>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>

namespace bibstd::app_framework
{

///
/// Static main active_worker class.
/// \warning Start called from parent thread.
/// \tparam ID of type std:size_t
///
template<std::size_t ID>
class active_worker final
{
public: // Constants
  static constexpr std::string_view log_channel = "active_worker";

public: // Accessor
  ///
  /// Get active_worker thread id.
  /// \return active_worker thread id if running, std::nullopt if not
  ///
  static auto worker_id() -> std::optional<std::thread::id>;

public: // Modifiers
  ///
  /// Start active_worker thread. Function will return after init function has returned.
  /// \param init Initial task to run in worker thread.
  /// \return guard to cleanup worker on guards destruction
  ///
  [[nodiscard]] static auto start(task_queue::task_type&& init = nullptr) -> util::scoped_guard;

  ///
  /// Queue task in active_worker thread.
  /// \param task Task that shall be run in active_worker thread.
  ///
  static auto queue_task(task_queue::task_type&& task) -> void;

  ///
  /// Run task in active_worker thread.
  /// \param task Task that shall be run in active_worker thread.
  ///
  static auto run_task(task_queue::task_type&& task) -> void;

private: // Implementation
  ///
  /// Stop main active_worker thread and cleanup task queues.
  ///
  static auto shutdown() -> void;

private: // Variables
  inline static std::atomic<std::optional<std::thread::id>> worker_id_{std::nullopt};
  inline static std::unique_ptr<task_queue> worker_queue_ = std::make_unique<task_queue>();
  inline static std::jthread worker_ = std::jthread();
};

///
/// In application used workers.
///
#include BOOST_PP_UPDATE_COUNTER()
using active_worker_main = active_worker<BOOST_PP_COUNTER>;

///
///
template<std::size_t ID>
auto active_worker<ID>::worker_id() -> std::optional<std::thread::id>
{
  return worker_id_.load();
}

///
///
template<std::size_t ID>
auto active_worker<ID>::start(task_queue::task_type&& init) -> util::scoped_guard
{
  std::promise<void> thread_initialized;
  auto future = thread_initialized.get_future();
  worker_ = std::jthread(
    [init_function = std::forward<decltype(init)>(init), initialized = std::move(thread_initialized)](std::stop_token stop_token) mutable
    {
      if(init_function)
      {
        init_function();
      }
      initialized.set_value();
      while(!stop_token.stop_requested())
      {
        try
        {
          while(!stop_token.stop_requested())
          {
            worker_queue_->empty() ? std::this_thread::sleep_for(std::chrono::milliseconds(100)) : worker_queue_->try_do_task();
          }
        }
        catch(const std::exception& e)
        {
          LOG_ERROR(log_channel, "Worker queue error: {}", e.what());
        }
        catch(...)
        {
          LOG_ERROR(log_channel, "Worker queue error: {}", "unknown exception");
        }
      }
    });
  worker_id_ = worker_.get_id();
  future.get();
  LOG_INFO(log_channel, "Init active_worker<{}> thread: id={}", ID, worker_.get_id());
  assert(std::this_thread::get_id() != worker_.get_id());
  return util::scoped_guard([]() { shutdown(); });
}

///
///
template<std::size_t ID>
auto active_worker<ID>::queue_task(task_queue::task_type&& task) -> void
{
  worker_queue_->queue(std::forward<decltype(task)>(task));
}

///
///
template<std::size_t ID>
auto active_worker<ID>::run_task(task_queue::task_type&& task) -> void
{
  if(std::this_thread::get_id() == active_worker<ID>::worker_id())
  {
    task();
  }
  else
  {
    active_worker<ID>::queue_task(std::forward<decltype(task)>(task));
  }
}

///
///
template<std::size_t ID>
auto active_worker<ID>::shutdown() -> void
{
  assert(std::this_thread::get_id() != worker_.get_id());
  LOG_INFO(log_channel, "Stop active_worker<{}> thread: id={}", ID, worker_.get_id());
  worker_id_ = std::nullopt;
  worker_.request_stop();
  worker_.join();
  worker_queue_->clear();
}

} // namespace bibstd::app_framework
