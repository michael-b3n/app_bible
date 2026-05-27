#include "bibstd/signal/synchronized_executor.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/log.hpp"
#include <algorithm>

namespace bibstd::signal
{

///
///
synchronized_executor::synchronized_executor()
  : thread_pool_guard_{framework::thread_pool::init()}
{
}

///
///
synchronized_executor::synchronized_executor(const framework::thread_pool::strand_id_type strand_id)
  : thread_pool_guard_{framework::thread_pool::init()}
  , strand_id_{strand_id}
{
}

///
///
synchronized_executor::~synchronized_executor() noexcept
{
  try
  {
    disconnect();
  }
  catch(...)
  {
    LOG_ERROR("synchronized_executor destruction exception: {}", util::exception_report());
  }
}

///
///
auto synchronized_executor::disconnect() -> void
{
  // Holding this lock until this functions returns (even though all the sync->mtx
  // locks are also locked afterwards) guarantees that this function cannot be
  // called again before it returns from another thread.
  const auto lock = std::scoped_lock{mtx_};
  connections_.clear();
  auto syncs = std::move(syncs_);
  syncs_ = {};
  std::ranges::for_each(
    syncs,
    [](const auto& sync)
    {
      const auto lock = std::scoped_lock{sync->mtx};
      sync->disconnected = true;
    }
  );
}

///
///
auto synchronized_executor::exec(framework::thread_pool::task_type&& task) const -> void
{
  if(strand_id_.has_value())
  {
    framework::thread_pool::queue_task(std::move(task), *strand_id_);
  }
  else
  {
    framework::thread_pool::queue_task(std::move(task));
  }
}

} // namespace bibstd::signal
