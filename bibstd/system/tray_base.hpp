#pragma once

#include "util/scoped_guard.hpp"

#include <cstddef>
#include <span>
#include <string>
#include <variant>

namespace bibstd::system
{

///
/// Tray base class for all system implementations.
///
struct tray_base
{
  // Typedefs
  ///
  /// Icon file loaded into memory.
  /// \param buffer Byte buffer view on a `*.ico` file
  ///
  struct icon_buffer final
  {
    std::span<const std::byte> buffer;
  };

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
};

} // namespace bibstd::system
