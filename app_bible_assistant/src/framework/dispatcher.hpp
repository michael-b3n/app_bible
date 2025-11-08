#pragma once

#include <bibstd/util/scoped_guard.hpp>

#include <functional>
#include <memory>
#include <mutex>

namespace bible_assistant::framework
{
namespace detail
{
// Forward declarations
class dispatcher_helper;
} // namespace detail

///
/// Dispatcher class to handle functions in main (UI) thread.
///
struct dispatcher final
{
  // Static Functions
  ///
  /// Initialize the dispatcher. This has to be called from the main (UI) thread.
  /// \return scoped guard to automatically shutdown the dispatcher on destruction
  ///
  static auto init() -> bibstd::util::scoped_guard;

  ///
  /// Function to run a task in the UI thread.
  /// The task is executed in the UI thread, which is usually the main thread.
  /// \param task Task to run in the UI thread
  ///
  static auto run_in_ui_thread(std::move_only_function<void()>&& task) -> void;

private: // Variables
  inline static std::mutex mtx_;
  static std::unique_ptr<detail::dispatcher_helper> helper_;
};

} // namespace bible_assistant::framework
