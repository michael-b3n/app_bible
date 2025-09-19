#pragma once

#include <concepts>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace bibstd::util
{

///
/// A thread-safe queue for copyable/movable tasks.
/// This class provides a queue that can be safely accessed from multiple threads.
/// It supports adding tasks (by copy or move), non-blocking and blocking pop operations,
/// and querying the queue's state (empty, size).
/// \tparam T Task type (must be copyable or movable)
///
template<typename T>
class concurrent_queue
{
public: // Structors
  concurrent_queue() = default;
  ~concurrent_queue() noexcept;

  ///
  /// Add a copy of a task to the queue.
  /// \param task The task to add (copied).
  ///
  auto push(const T& task) -> void
    requires std::copyable<T>;

  ///
  /// Add a task to the queue using move semantics.
  /// \param task The task to add (moved).
  ///
  auto push(T&& task) -> void
    requires std::movable<T>;

  ///
  /// Try to pop a task from the queue.
  /// \return optional task if available, std::nullopt otherwise
  ///
  auto try_pop() -> std::optional<T>;

  ///
  /// Wait until a task is available and pop it from the queue.
  /// \return optional next task in the queue, std::nullopt if no task is available within the timeout
  ///
  auto wait_and_pop(std::chrono::milliseconds timeout = std::chrono::minutes(10)) -> std::optional<T>;

  ///
  /// Check if the queue is empty.
  /// \return true if the queue is empty, false otherwise
  ///
  auto empty() const -> bool;

  ///
  ///  Get the number of tasks in the queue.
  /// \return size of the queue
  ///
  auto size() const -> std::size_t;

private:
  mutable std::mutex mtx_;
  bool shutdown_{false};
  std::condition_variable cv_;
  std::queue<T> queue_;
};

///
///
template<typename T>
concurrent_queue<T>::~concurrent_queue() noexcept
{
  {
    const auto lock = std::lock_guard(mtx_);
    shutdown_ = true;
  }
  while(!empty()) // clear queue on shutdown
  {
    std::ignore = try_pop();
  }
  cv_.notify_all(); // Notify all waiting threads to wake up and exit
}

///
///
template<typename T>
auto concurrent_queue<T>::push(const T& task) -> void
  requires(std::copyable<T>)
{
  {
    const auto lock = std::lock_guard(mtx_);
    if(shutdown_)
    {
      return;
    }
    queue_.push(task);
  }
  cv_.notify_one();
}

///
///
template<typename T>
auto concurrent_queue<T>::push(T&& task) -> void
  requires(std::movable<T>)
{
  {
    const auto lock = std::lock_guard(mtx_);
    if(shutdown_)
    {
      return;
    }
    queue_.push(std::move(task));
  }
  cv_.notify_one();
}

///
///
template<typename T>
auto concurrent_queue<T>::try_pop() -> std::optional<T>
{
  const auto lock = std::lock_guard(mtx_);
  if(shutdown_ || queue_.empty())
  {
    return std::nullopt;
  }
  T task = std::move(queue_.front());
  queue_.pop();
  return std::optional<T>{std::move(task)};
}

///
///
template<typename T>
auto concurrent_queue<T>::wait_and_pop(const std::chrono::milliseconds timeout) -> std::optional<T>
{
  auto lock = std::unique_lock(mtx_);
  if(!cv_.wait_for(lock, timeout, [this] { return shutdown_ || !queue_.empty(); }))
  {
    return std::nullopt;
  }
  if(shutdown_)
  {
    return std::nullopt;
  }
  T task = std::move(queue_.front());
  queue_.pop();
  return std::optional<T>{std::move(task)};
}

///
///
template<typename T>
auto concurrent_queue<T>::empty() const -> bool
{
  const auto lock = std::lock_guard(mtx_);
  return queue_.empty();
}

///
///
template<typename T>
auto concurrent_queue<T>::size() const -> std::size_t
{
  std::lock_guard<std::mutex> lock(mtx_);
  return queue_.size();
}

} // namespace bibstd::util
