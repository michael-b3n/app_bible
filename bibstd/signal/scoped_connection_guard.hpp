
#pragma once

#include "bibstd/signal/common.hpp"
#include "bibstd/signal/connection_store.hpp"
#include "bibstd/util/log.hpp"

#include <memory>
#include <mutex>
#include <shared_mutex>

namespace bibstd::signal
{

///
/// Scoped connection guard that stores connections to signals
/// and disconnects them when destroyed. This guard is thread-safe
/// and guarantees that any slots connected through this guard will
/// not be called after disconnect returns, even if they are just
/// about to be called.
///
class scoped_connection_guard final
{
public: // Constructors
  scoped_connection_guard() = default;
  inline ~scoped_connection_guard() noexcept;

public: // Operations
  ///
  /// Connects a slot to a signal and stores the connection in the guard.
  /// The slot will be automatically disconnected when the guard is destroyed or disconnect is called.
  /// \param sig Signal to connect to
  /// \param slot Slot to connect
  ///
  template<signal_like T>
  auto connect(T& sig, typename T::slot_type slot) -> void;

  ///
  /// Connects an extended slot to a signal and stores the connection in the guard.
  /// The extended slot will be automatically disconnected when the guard is destroyed or disconnect is called.
  /// \param sig Signal to connect to
  /// \param slot Extended slot to connect
  ///
  template<signal_like T>
  auto connect_extended(T& sig, typename T::extended_slot_type slot) -> void;

  ///
  /// Disconnects all connections in the guard. Any slots connected through this guard
  /// will not be called after disconnect returns, even if they are still connected to
  /// their signals, being called or just about to be called.
  ///
  inline auto disconnect() -> void;

private: // Typedefs
  struct sync_wrapper final
  {
    bool disconnected{false};
    std::shared_mutex mtx{};
  };

private: // Variables
  mutable std::mutex mtx_;
  std::shared_ptr<sync_wrapper> sync_{std::make_shared<sync_wrapper>()};
  connection_store connections_;
};

///
///
scoped_connection_guard::~scoped_connection_guard() noexcept
{
  try
  {
    disconnect();
  }
  catch(const std::exception& e)
  {
    LOG_ERROR("scoped connection guard destruction exception: {}", e.what());
  }
  catch(...)
  {
    LOG_ERROR("scoped connection guard destruction exception: {}", "unknown exception");
  }
}

///
///
template<signal_like T>
auto scoped_connection_guard::connect(T& sig, typename T::slot_type slot) -> void
{
  const auto lock = std::scoped_lock{mtx_};
  typename T::slot_type wrapped_slot = [sync = sync_, slot = std::move(slot)](auto&&... args)
  {
    const auto lock = std::shared_lock{sync->mtx};
    if(sync->disconnected)
    {
      return;
    }
    slot(std::forward<decltype(args)>(args)...);
  };
  connections_.store(sig.connect(std::move(wrapped_slot)));
}

///
///
template<signal_like T>
auto scoped_connection_guard::connect_extended(T& sig, typename T::extended_slot_type slot) -> void
{
  const auto lock = std::scoped_lock{mtx_};
  typename T::extended_slot_type wrapped_slot = [sync = sync_, slot = std::move(slot)](auto&&... args)
  {
    const auto lock = std::shared_lock{sync->mtx};
    if(sync->disconnected)
    {
      return;
    }
    return slot(std::forward<decltype(args)>(args)...);
  };
  connections_.store(sig.connect_extended(std::move(wrapped_slot)));
}

///
///
auto scoped_connection_guard::disconnect() -> void
{
  const auto lock = std::scoped_lock{mtx_};
  connections_.clear();
  auto sync = sync_;
  sync_ = std::make_shared<sync_wrapper>();
  {
    const auto lock = std::unique_lock{sync->mtx};
    sync->disconnected = true;
  }
}

} // namespace bibstd::signal
