#pragma once

#include "bibstd/signal/common.hpp"

#include <concepts>
#include <functional>
#include <type_traits>

namespace bibstd::signal
{

///
/// Concept for executors that can be used with the adapter.
///
template<typename T>
concept executor_kind = requires(T t, std::move_only_function<void()> task) {
  { t(std::move(task)) } -> std::same_as<void>;
  { t.store(scoped_connection_type{}) } -> std::same_as<void>;
};

///
/// Signal adapter class.
///
template<std::default_initializable T>
class adapter
{
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
  [[nodiscard]] auto connect(auto sig_projection, auto&& slot) -> scoped_connection_type;

  ///
  /// Connect a slot to the signal specified by the projection, allowing extended functionality.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \return connection corresponding to connected slot
  ///
  auto connect_extended(auto sig_projection, auto&& slot) -> connection_type;

  ///
  ///  Connect a slot to the signal specified by the projection, ensuring the slot is called
  /// in the context of the provided executor. The lifetime of the executor must exceed the lifetime of the slot. The slot
  /// connection is bound to the lifetime of the executor.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \param executor Executor to use for the slot
  ///
  auto connect_queued(auto sig_projection, auto&& slot, executor_kind auto& executor) -> void;

  ///
  /// Connect a slot to the signal specified by the projection, allowing extended functionality and ensuring
  /// the slot is called in the context of the provided executor. The lifetime of the executor must exceed the lifetime of the
  /// slot. The slot connection is bound to the lifetime of the executor.
  /// \param sig_projection Projection to the signal to connect
  /// \param slot Slot to connect
  /// \param executor Executor to use for the slot
  ///
  auto connect_queued_extended(auto sig_projection, auto&& slot, executor_kind auto& executor) -> void;

protected: // Accessors
  ///
  /// Emit the signal specified by the projection, passing the provided arguments to the connected slots.
  /// \param sig_projection Projection to the signal to emit
  /// \param args Arguments to pass to the connected slots
  /// \return result of the signal call, if any
  ///
  auto notify(auto sig_projection, auto&&... args) -> auto;

private: // Variables
  T sigs_;
};

///
///
template<std::default_initializable T>
auto adapter<T>::connect(auto sig_projection, auto&& slot) -> scoped_connection_type
{
  return std::invoke(sig_projection, sigs_).connect(std::forward<decltype(slot)>(slot));
}

///
///
template<std::default_initializable T>
auto adapter<T>::connect_extended(auto sig_projection, auto&& slot) -> connection_type
{
  return std::invoke(sig_projection, sigs_).connect_extended(std::forward<decltype(slot)>(slot));
}

///
///
template<std::default_initializable T>
auto adapter<T>::connect_queued(auto sig_projection, auto&& slot, executor_kind auto& executor) -> void
{
  using sig_type = std::remove_cvref_t<std::invoke_result_t<decltype(sig_projection), T&>>;
  using function_type = to_functional<sig_type, std::function>::type;
  static_assert(std::is_convertible_v<decltype(slot), function_type>);

  function_type func = [slot = std::forward<decltype(slot)>(slot), &executor](auto&&... args)
  {
    executor([slot = std::move(slot), args = std::make_tuple(std::forward<decltype(args)>(args)...)]() mutable
             { std::apply(std::move(slot), std::move(args)); });
  };
  executor.store(std::invoke(sig_projection, sigs_).connect(std::move(func)));
}

///
///
template<std::default_initializable T>
auto adapter<T>::connect_queued_extended(auto sig_projection, auto&& slot, executor_kind auto& executor) -> void
{
  using sig_type = std::remove_cvref_t<std::invoke_result_t<decltype(sig_projection), T&>>;
  using function_type = to_functional<sig_type, std::function>::type_extended;
  static_assert(std::is_convertible_v<decltype(slot), function_type>);

  function_type func = [slot = std::forward<decltype(slot)>(slot), &executor](auto&&... args)
  {
    executor([slot = std::move(slot), args = std::make_tuple(std::forward<decltype(args)>(args)...)]() mutable
             { std::apply(std::move(slot), std::move(args)); });
  };
  executor.store(std::invoke(sig_projection, sigs_).connect_extended(std::move(func)));
}

///
///
template<std::default_initializable T>
auto adapter<T>::notify(auto sig_projection, auto&&... args) -> auto
{
  return std::invoke(sig_projection, sigs_)(std::forward<decltype(args)>(args)...);
}

} // namespace bibstd::signal
