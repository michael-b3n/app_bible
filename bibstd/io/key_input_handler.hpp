#pragma once

#include "util/log.hpp"

#include <functional>

namespace bibstd::io
{

class key_input_handler
{
public: // Typedefs
  using key_id_type = unsigned int;

  enum class key
  {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8,
    _9,
    _0,
  };

  enum class key_modifier
  {
    alt,
    control,
    shift,
    alt_control,
    alt_shift,
    control_shift,
  };

public: // Constructor
  key_input_handler();

public: // Modifiers
  auto register_global_key(key_id_type key_id, key_modifier mod, std::function<void()>&& callback) -> void;
};

} // namespace bibstd::io
