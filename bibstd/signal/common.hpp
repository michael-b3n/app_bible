#pragma once

#include <boost/signals2.hpp>

#include <catch2/catch_test_macros.hpp>
#include <functional>
#include <mutex>
#include <vector>

namespace bibstd::signal
{

///
/// Type alias for signal type from boost::signals2.
/// A signal can be used to connect slots (callback functions) to it.
/// Calling a signal will call all connected slots.
///
template<typename T>
using signal_type = boost::signals2::signal<T>;

///
/// Type trait to check if a type is a signal type.
///
template<typename T>
struct is_signal_type final : std::false_type
{};
template<typename T>
struct is_signal_type<signal_type<T>> final : std::true_type
{};

///
/// Boolean flag to check if a type is a signal type.
/// \see is_signal_type
///
template<typename T>
inline constexpr auto is_signal_type_v = is_signal_type<std::remove_cvref_t<T>>::value;

namespace detail
{

///
/// Helper for `signal_signature` to extract signal signature from a signal type.
///
template<typename T>
struct signal_signature_extractor;
template<typename R, typename... Args>
struct signal_signature_extractor<R(Args...)> final
{
  using return_type = R;
  using args_tuple = std::tuple<Args...>;
};

} // namespace detail

///
/// Type trait to access signal signature.
/// The signal signature consists of the return type and the argument types of the signal.
///
template<typename T>
  requires(is_signal_type_v<T>)
struct signal_signature
{
private:
  using __signature = detail::signal_signature_extractor<typename T::slot_type::signature_type>;
  using __signature_extended = detail::signal_signature_extractor<typename T::extended_slot_type::signature_type>;
  static_assert(std::is_same_v<typename __signature::return_type, typename __signature_extended::return_type>);

public:
  using return_type = typename __signature::return_type;
  using args_tuple = typename __signature::args_tuple;
  using extended_args_tuple = typename __signature_extended::args_tuple;
};

///
/// Type trait to convert a signal type to a functional type (std::function or move_only_function).
/// Provides type aliases for the corresponding functional types for both the regular and extended slot types of the signal.
/// \tparam T signal type to convert to functional type
/// \tparam F functional type to convert to (std::function or move_only_function)
///
template<typename T, template<typename...> typename F>
  requires(is_signal_type_v<T>)
struct to_functional;
template<typename T>
struct to_functional<T, std::function> final
{
  using type = std::function<typename T::slot_type::signature_type>;
  using type_extended = std::function<typename T::extended_slot_type::signature_type>;
};
template<typename T>
struct to_functional<T, std::move_only_function> final
{
  using type = std::move_only_function<typename T::slot_type::signature_type>;
  using type_extended = std::move_only_function<typename T::extended_slot_type::signature_type>;
};

///
/// Type alias for connection type from boost::signals2.
/// A connection can be used to disconnect a slot from a signal.
///
using connection_type = boost::signals2::connection;

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
  auto store(scoped_connection_type&& con) -> void;

  ///
  /// Clear connection store and disconnect all connections.
  ///
  auto clear() -> void;

private: // Variables
  mutable std::mutex mtx_;
  std::vector<scoped_connection_type> connections_;
};

} // namespace bibstd::signal
