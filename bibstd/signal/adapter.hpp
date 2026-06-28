#pragma once

#include "bibstd/signal/common.hpp"

#include <concepts>
#include <functional>

namespace bibstd::signal
{
namespace detail
{
// clang-format off
struct _any_return_type{};
struct _any_argument_type{};
using _any_signal_type = signal_type<_any_return_type(_any_argument_type)>;
// clang-format on
} // namespace detail

///
/// Concept for executors that can be used with the adapter.
///
template<typename T>
concept executor_kind = requires(
  T t,
  detail::_any_signal_type& sig,
  detail::_any_signal_type::slot_type slot,
  detail::_any_signal_type::extended_slot_type ext_slot
) {
  { t.connect(sig, std::move(slot)) } -> std::same_as<void>;
  { t.connect_extended(sig, std::move(ext_slot)) } -> std::same_as<void>;
};

///
/// Signal adapter class.
///
template<std::default_initializable T>
class adapter
{
  // Variables
  mutable T sigs_; // mutable to be able to fire and connect from constant members

public: // Typedefs
  using signals_type = T;

public: // Structors
  adapter() = default;
  virtual ~adapter() noexcept = default;

public: // Modifiers
  ///
  /// Connect a slot to the signal specified by the projection.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \return scoped connection corresponding to connected slot
  ///
  [[nodiscard]] auto connect(auto sig_projection, auto&& slot) const -> scoped_connection_type;

  ///
  /// Connect a slot to the signal specified by the projection, allowing extended functionality.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \return connection corresponding to connected slot
  ///
  auto connect_extended(auto sig_projection, auto&& slot) const -> connection_type;

  ///
  ///  Connect a slot to the signal specified by the projection, ensuring the slot is called
  /// in the context of the provided executor. The lifetime of the executor must exceed the lifetime of the slot. The slot
  /// connection is bound to the lifetime of the executor.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \param executor Executor to use for the slot
  ///
  auto connect_queued(auto sig_projection, auto&& slot, executor_kind auto& executor) const -> void;

  ///
  /// Connect a slot to the signal specified by the projection, allowing extended functionality and ensuring
  /// the slot is called in the context of the provided executor. The lifetime of the executor must exceed the lifetime of the
  /// slot. The slot connection is bound to the lifetime of the executor.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \param executor Executor to use for the slot
  ///
  auto connect_queued_extended(auto sig_projection, auto&& slot, executor_kind auto& executor) const -> void;

protected: // Accessors
  ///
  /// Emit the signal specified by the projection, passing the provided arguments to the connected slots.
  /// \param sig_projection Projection to the signal to emit
  /// \param args Arguments to pass to the connected slots
  /// \return result of the signal call, if any
  ///
  auto notify(auto sig_projection, auto&&... args) -> auto;
};

///
///
template<std::default_initializable T>
auto adapter<T>::connect(auto sig_projection, auto&& slot) const -> scoped_connection_type
{
  return std::invoke(sig_projection, sigs_).connect(std::forward<decltype(slot)>(slot));
}

///
///
template<std::default_initializable T>
auto adapter<T>::connect_extended(auto sig_projection, auto&& slot) const -> connection_type
{
  return std::invoke(sig_projection, sigs_).connect_extended(std::forward<decltype(slot)>(slot));
}

///
///
template<std::default_initializable T>
auto adapter<T>::connect_queued(auto sig_projection, auto&& slot, executor_kind auto& executor) const -> void
{
  executor.connect(std::invoke(sig_projection, sigs_), std::forward<decltype(slot)>(slot));
}

///
///
template<std::default_initializable T>
auto adapter<T>::connect_queued_extended(auto sig_projection, auto&& slot, executor_kind auto& executor) const -> void
{
  executor.connect_extended(std::invoke(sig_projection, sigs_), std::forward<decltype(slot)>(slot));
}

///
///
template<std::default_initializable T>
auto adapter<T>::notify(auto sig_projection, auto&&... args) -> auto
{
  return std::invoke(sig_projection, sigs_)(std::forward<decltype(args)>(args)...);
}

} // namespace bibstd::signal
