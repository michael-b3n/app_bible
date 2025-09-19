#pragma once

#include "meta/pack.hpp"
#include "meta/type_traits.hpp"
#include "util/signals.hpp"

#include <concepts>
#include <string_view>

namespace bibstd::framework
{

///
/// Common signal IDs. They shall be used in various adapters.
///
enum class common_signal_id
{
  queued,
  started,
  progress,
  ended,
  applied,
  changed,
};

///
///
///
template<auto ID, typename T>
struct named_signal;

///
/// Named signal specification
///
template<auto ID, typename T>
struct named_signal<ID, util::signal_type<T>>
{
  // Typedefs
  using signal_id = decltype(ID);
  using signal_type = util::signal_type<T>;

  // Constants
  static constexpr auto id = ID;

  // Variables
  signal_type signal;
};

namespace detail
{

template<typename T>
struct is_named_signal : std::false_type
{};
template<auto I, typename T>
struct is_named_signal<named_signal<I, util::signal_type<T>>> : std::true_type
{};
template<typename T>
concept signal_adapter_arg = is_named_signal<T>::value;

} // namespace detail

///
/// Signal adapter class.
///
template<detail::signal_adapter_arg... Args>
class signal_adapter
{
  static_assert(meta::are_same_v<typename Args::signal_id...>, "registered signals must have same ID types");

public: // Typedefs
  using signal_id = meta::pack<Args...>::first_type::signal_id;

public: // Constants
  static constexpr auto registered_ids = std::array{Args::id...};

public: // Modifiers
  ///
  /// Connect a slot to the signal with the specified ID.
  /// \param slot Slot to connect
  /// \return scoped connection corresponding to connected slot
  ///
  template<signal_id ID, typename Slot>
  [[nodiscard]] auto connect(Slot&& slot) -> util::scoped_connection_type;

  ///
  /// Connect a slot to the signal with the specified ID, allowing extended functionality.
  /// \param slot Slot to connect
  /// \return connection corresponding to connected slot
  ///
  template<signal_id ID, typename Slot>
  auto connect_extended(Slot&& slot) -> util::connection_type;

protected: // Accessors
  ///
  /// Emit the signal with the specified ID, passing the provided arguments to the connected slots.
  /// \param args Arguments to pass to the connected slots
  /// \return result of the signal call, if any
  ///
  template<signal_id ID, typename... SignalArgs>
  auto emit(SignalArgs... args) -> auto;

private: // Implementation
  template<signal_id ID>
  consteval auto find_index() const -> std::size_t;

private: // Variables
  std::tuple<Args...> signals_;
};

///
///
template<detail::signal_adapter_arg... Args>
template<signal_adapter<Args...>::signal_id ID, typename Slot>
auto signal_adapter<Args...>::connect(Slot&& slot) -> util::scoped_connection_type
{
  constexpr auto index = find_index<ID>();
  return std::get<index>(signals_).signal.connect(std::forward<Slot>(slot));
}

///
///
template<detail::signal_adapter_arg... Args>
template<signal_adapter<Args...>::signal_id ID, typename Slot>
auto signal_adapter<Args...>::connect_extended(Slot&& slot) -> util::connection_type
{
  constexpr auto index = find_index<ID>();
  return std::get<index>(signals_).signal.connect_extended(std::forward<Slot>(slot));
}

///
///
template<detail::signal_adapter_arg... Args>
template<signal_adapter<Args...>::signal_id ID, typename... SignalArgs>
auto signal_adapter<Args...>::emit(SignalArgs... args) -> auto
{
  constexpr auto index = find_index<ID>();
  return std::get<index>(signals_).signal(args...);
}

///
///
template<detail::signal_adapter_arg... Args>
template<signal_adapter<Args...>::signal_id ID>
consteval auto signal_adapter<Args...>::find_index() const -> std::size_t
{
  constexpr auto find_index = [&]<std::size_t I>(std::size_t& retval_index)
  {
    if constexpr(std::tuple_element_t<I, decltype(signals_)>::id == ID)
    {
      retval_index = I;
    }
  };
  constexpr auto index = [&]<std::size_t... I>(std::index_sequence<I...>)
  {
    auto retval = std::numeric_limits<std::size_t>::max();
    (find_index.template operator()<I>(retval), ...);
    return retval;
  }(std::make_index_sequence<std::tuple_size_v<decltype(signals_)>>{});
  static_assert(index < std::tuple_size_v<decltype(signals_)>, "no index found matching specified ID");
  return index;
}

} // namespace bibstd::framework
