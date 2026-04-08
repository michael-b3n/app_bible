#pragma once

#include "bibstd/framework/thread_pool.hpp"
#include "bibstd/signal/common.hpp"
#include "bibstd/signal/connection_store.hpp"
#include "bibstd/util/scope_guard.hpp"

namespace bibstd::signal
{

///
/// Executor used for executing signals in the default thread pool.
/// \see signal::adapter::connect_queued and signal::adapter::connect_queued_extended
///
class executor final
{
public: // Constructor
  executor();
  executor(framework::thread_pool::strand_id_type strand_id);

public: // Operators
  ///
  /// Execute task in thread pool. If strand_id was specified in constructor,
  /// the task will be executed in the corresponding strand.
  /// \param task Task to execute
  ///
  auto operator()(framework::thread_pool::task_type&& task) const -> void;

public: // Modifiers
  ///
  /// Add connection to connection store. The connection will be automatically disconnected
  /// when the executor is destroyed. This allows lifetime syncronization for queued slots.
  /// \see adapter::connect_queued and adapter::connect_queued_extended
  /// \param con Connection that shall be added
  ///
  auto store(scoped_connection_type&& con) -> void;

private: // Variables
  const util::shared_scope_guard thread_pool_guard_;
  std::optional<framework::thread_pool::strand_id_type> strand_id_;
  connection_store connections_;
};

} // namespace bibstd::signal
