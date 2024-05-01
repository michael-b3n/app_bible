#pragma once

#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <variant>

// Forward declaration
namespace Tray
{
class Tray;
}; // namespace Tray

namespace bibstd::system
{

///
/// Tray wrapper class.
///
class tray final
{
public: // Typedefs
  // clang-format off
  struct button final { std::string text; std::function<void()> callback; };
  struct label final { std::string text; };
  struct separator final {};
  struct toggle final { std::string text; bool state; std::function<void(bool)> callback; };
  // clang-format on
  struct submenu final
  {
    using entry_type = std::variant<button, label, separator, toggle>;
    std::string text;
    std::vector<entry_type> entries;
  };

public: // Constants
  static constexpr std::string_view log_channel = "tray";

public: // Structors
  tray(std::string identifier, std::filesystem::path&& icon_path);
  ~tray() noexcept;

public: // Operations
  auto reset() -> void;
  auto update() -> void;

public: // Modifiers
  ///
  /// Add exit button to tray that invalidates `this` type on call.
  /// \param button Name and callable that shall be called on button click
  ///
  auto add_exit_button(button&& button) -> void;

  auto add_button(button&& button) -> void;
  auto add_label(label&& label) -> void;
  auto add_separator() -> void;
  auto add_toggle(toggle&& toggle) -> void;
  auto add_submenu(submenu&& submenu) -> void;

private: // Variables
  mutable std::mutex mtx_;
  const std::string identifier_;
  std::unique_ptr<Tray::Tray> tray_;
};

} // namespace bibstd::system
