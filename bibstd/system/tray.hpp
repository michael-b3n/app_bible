#pragma once

#include "bibstd/framework/active_worker.hpp"
#include "bibstd/util/scoped_guard.hpp"

#include <tray.hpp>

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace bibstd::system
{

///
/// Tray class for windows implementation.
///
class tray final
{
public: // Typedefs
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

  using entry_type = std::variant<button, label, separator, toggle>;

public: // Static modifiers
  static auto init(icon_buffer icon, std::vector<entry_type>&& entries) -> util::scoped_guard;

private: // Static helpers
  static auto get_message() -> void;

private:
  inline static std::unique_ptr<framework::active_worker> worker_{};
  inline static std::unique_ptr<Tray::Tray> tray_{nullptr};
  inline static std::map<int, std::function<void()>> callback_map_{};
};

} // namespace bibstd::system
