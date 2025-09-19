#include "framework/dispatcher.hpp"

#include <util/concurrent_queue.hpp>
#include <util/log.hpp>

#include <QGuiApplication>
#include <QObject>

#include <atomic>
#include <functional>

namespace bible_assistant::framework
{
namespace detail
{

////
/// The dispatcher_helper internally stores the functions to be called in the UI thread.
/// It sends a signal itself and the slot extracts the function from the list and calls it.
///
class dispatcher_helper final : public QObject
{
  Q_OBJECT

public: // Typedefs
  using task_type = std::move_only_function<void()>;

public: // Structors
  dispatcher_helper();
  virtual ~dispatcher_helper() noexcept;

public: // Operations
  ///
  /// Run task in UI thread.
  /// \param task Function function to run
  ///
  auto run_in_ui_thread(task_type&& task) -> void;

signals: // QObject Signals
  auto signal_to_ui_thread() -> void;

private slots: //  QObject Slots
  auto slot_in_ui_thread() -> void;

private:

private:
  std::atomic_bool running_{true};
  bibstd::util::concurrent_queue<task_type> task_queue_;
};

///
///
dispatcher_helper::dispatcher_helper()
  : QObject(nullptr)
{
  moveToThread(QGuiApplication::instance()->thread());
  // Connect signal to slot
  connect(this, &dispatcher_helper::signal_to_ui_thread, this, &dispatcher_helper::slot_in_ui_thread, Qt::QueuedConnection);
}

///
///
dispatcher_helper::~dispatcher_helper() noexcept
{
  running_ = false;
  disconnect(this, &dispatcher_helper::signal_to_ui_thread, this, &dispatcher_helper::slot_in_ui_thread);
}

///
///
auto dispatcher_helper::run_in_ui_thread(task_type&& task) -> void
{
  if(!running_ || !task)
  {
    return;
  }
  task_queue_.push(std::move(task));
  emit signal_to_ui_thread();
}

///
///
auto dispatcher_helper::slot_in_ui_thread() -> void
{
  if(!running_)
  {
    return;
  }
  auto task = task_queue_.try_pop();
  if(task && *task)
  {
    try
    {
      (*task)();
    }
    catch(const std::exception& e)
    {
      // Log error
      LOG_ERROR("exception in ui thread task: {}", e.what());
    }
    catch(...)
    {
      LOG_ERROR("unknown exception in UI thread task");
    }
  }
}

} // namespace detail

///
///
std::unique_ptr<detail::dispatcher_helper> dispatcher::helper_{nullptr};

///
///
auto dispatcher::init() -> bibstd::util::scoped_guard
{
  const auto lock = std::lock_guard(mtx_);
  if(!helper_)
  {
    helper_ = std::make_unique<detail::dispatcher_helper>();
  }
  return bibstd::util::scoped_guard(
    []()
    {
      const auto lock = std::lock_guard(mtx_);
      helper_.reset();
    }
  );
}

///
///
auto dispatcher::run_in_ui_thread(std::move_only_function<void()>&& task) -> void
{
  const auto lock = std::lock_guard(mtx_);
  if(helper_)
  {
    helper_->run_in_ui_thread(std::move(task));
  }
}

} // namespace bible_assistant::framework

#include "dispatcher.moc"
