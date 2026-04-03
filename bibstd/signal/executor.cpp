#include "bibstd/signal/executor.hpp"

namespace bibstd::signal
{

///
///
executor::executor(const framework::thread_pool::strand_id_type strand_id)
  : strand_id_{strand_id}
{
}

///
///
auto executor::operator()(framework::thread_pool::task_type&& task) const -> void
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

///
///
auto executor::store(scoped_connection_type&& con) -> void
{
  connections_.store(std::move(con));
}

} // namespace bibstd::signal
