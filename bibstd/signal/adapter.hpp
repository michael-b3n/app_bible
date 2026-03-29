#pragma once

#include "bibstd/meta/pack.hpp"
#include "bibstd/meta/type_traits.hpp"
#include "bibstd/signal/common.hpp"

namespace bibstd::signal
{

///
/// Named signal template.
///
template<auto ID, typename T>
struct named_signal;

///
/// Named signal specification
///
template<auto ID, typename T>
struct named_signal<ID, signal_type<T>>
{
  // Constants
  static constexpr auto id = ID;

  // Variables
  signal_type<T> signal;
};

namespace detail
{

template<typename T>
struct is_named_signal : std::false_type
{};
template<auto I, typename T>
struct is_named_signal<named_signal<I, signal_type<T>>> : std::true_type
{};
template<typename T>
concept adapter_arg = is_named_signal<T>::value;

} // namespace detail

///
/// Signal adapter class.
///
template<detail::adapter_arg... Args>
class adapter
{
  static_assert(meta::are_same_v<decltype(Args::id)...>, "registered signals must have same ID types");

public: // Typedefs
  using signal_id = decltype(meta::pack<Args...>::first_type::id);

public: // Constants
  static constexpr auto registered_ids = std::array{Args::id...};

public: // Modifiers
  ///
  /// Connect a slot to the signal with the specified ID.
  /// \param slot Slot to connect
  /// \return scoped connection corresponding to connected slot
  ///
  template<signal_id ID, typename Slot>
  [[nodiscard]] auto connect(Slot&& slot) -> scoped_connection_type;

  ///
  /// Connect a slot to the signal with the specified ID, allowing extended functionality.
  /// \param slot Slot to connect
  /// \return connection corresponding to connected slot
  ///
  template<signal_id ID, typename Slot>
  auto connect_extended(Slot&& slot) -> connection_type;

protected: // Accessors
  ///
  /// Emit the signal with the specified ID, passing the provided arguments to the connected slots.
  /// \param args Arguments to pass to the connected slots
  /// \return result of the signal call, if any
  ///
  template<signal_id ID, typename... SignalArgs>
  auto notify(SignalArgs... args) -> auto;

private: // Implementation
  template<signal_id ID>
  consteval static auto find_index() -> std::size_t;

private: // Variables
  std::tuple<Args...> signals_;
};

///
///
template<detail::adapter_arg... Args>
template<adapter<Args...>::signal_id ID, typename Slot>
auto adapter<Args...>::connect(Slot&& slot) -> scoped_connection_type
{
  constexpr auto index = find_index<ID>();
  return std::get<index>(signals_).signal.connect(std::forward<Slot>(slot));
}

///
///
template<detail::adapter_arg... Args>
template<adapter<Args...>::signal_id ID, typename Slot>
auto adapter<Args...>::connect_extended(Slot&& slot) -> connection_type
{
  constexpr auto index = find_index<ID>();
  return std::get<index>(signals_).signal.connect_extended(std::forward<Slot>(slot));
}

///
///
template<detail::adapter_arg... Args>
template<adapter<Args...>::signal_id ID, typename... SignalArgs>
auto adapter<Args...>::notify(SignalArgs... args) -> auto
{
  constexpr auto index = find_index<ID>();
  return std::get<index>(signals_).signal(args...);
}

///
///
template<detail::adapter_arg... Args>
template<adapter<Args...>::signal_id ID>
consteval auto adapter<Args...>::find_index() -> std::size_t
{
  constexpr auto find_index = [&]<std::size_t I>(std::size_t& retval_index)
  {
    if constexpr(std::tuple_element_t<I, std::tuple<Args...>>::id == ID)
    {
      retval_index = I;
    }
  };
  constexpr auto index = [&]<std::size_t... I>(std::index_sequence<I...>)
  {
    auto retval = std::numeric_limits<std::size_t>::max();
    (find_index.template operator()<I>(retval), ...);
    return retval;
  }(std::make_index_sequence<std::tuple_size_v<std::tuple<Args...>>>{});
  static_assert(index < std::tuple_size_v<std::tuple<Args...>>, "no index found matching specified ID");
  return index;
}

} // namespace bibstd::signal
