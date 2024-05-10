#pragma once

#include "util/scoped_guard.hpp"

#include <variant>

namespace bibstd::system
{

///
/// Key input base class for all system implementations.
///
struct tray_base
{
  // Typedefs
  // clang-format off
  struct button final { std::string text; std::function<void()> callback; };
  struct label final { std::string text; };
  struct separator final {};
  struct toggle final { std::string text; bool state; std::function<std::function<void()>(bool)> callback; };
  // clang-format on
  struct submenu final
  {
    using simple_entry_type = std::variant<button, label, separator, toggle>;
    std::string text;
    std::vector<simple_entry_type> entries;
  };

  using entry_type = std::variant<button, label, separator, toggle, submenu>;

  // Constants
  static constexpr std::string_view log_channel = "tray";
};

} // namespace bibstd::system
