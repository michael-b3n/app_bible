#pragma once

#include "util/scoped_guard.hpp"

namespace bibstd::system
{

///
/// Key input base class for all system implementations.
///
struct hotkey_base
{
  // Typedefs
  enum class key
  {
    vk_a,
    vk_b,
    vk_c,
    vk_d,
    vk_e,
    vk_f,
    vk_g,
    vk_h,
    vk_i,
    vk_j,
    vk_k,
    vk_l,
    vk_m,
    vk_n,
    vk_o,
    vk_p,
    vk_q,
    vk_r,
    vk_s,
    vk_t,
    vk_u,
    vk_v,
    vk_w,
    vk_x,
    vk_y,
    vk_z,
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

  // Constants
  static constexpr std::string_view log_channel = "hotkey";
};

} // namespace bibstd::system
