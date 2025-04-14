#pragma once

#include <boost/signals2.hpp>

#include <mutex>
#include <vector>

namespace util
{

///
/// Type alias for signal type from boost::signals2.
/// A signal can be used to connect slots (callback functions) to it.
/// Calling a signal will call all connected slots.
///
template<typename T>
using signal_type = boost::signals2::signal<T>;

///
/// Boost::signals2 scoped connection type.
/// Disconnects the connection when it goes out of scope.
///
using scoped_connection_type = boost::signals2::scoped_connection;

///
/// Boost::signals2 shared connection blocker.
/// This is used to block a signal temporarily.
///
using connection_block_type = boost::signals2::shared_connection_block;

///
/// Connection store that stores connections to signals and disconnects them when destroyed.
///
class connection_store final
{
public: // Constructor
  connection_store() = default;
  ~connection_store() noexcept;

public: // Modifiers
  ///
  /// Add connection to connection store.
  /// \param con Connection that shall be added
  ///
  auto add_connection(scoped_connection_type&& con) -> void;

  ///
  /// Clear connection store and disconnect all connections.
  ///
  auto clear() -> void;

private: // Variables
  mutable std::mutex mtx_;
  std::vector<scoped_connection_type> connections_;
};

} // namespace util
