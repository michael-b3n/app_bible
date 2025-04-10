#pragma once

#include "util/exception.hpp"
#include "util/property.hpp"
#include "util/property_parser.hpp"

#include <filesystem>
#include <memory>
#include <mutex>
#include <string_view>

namespace bibstd::util
{

///
/// Property tree. This class manages saving and loading of properties.
/// \note This class must be created as a shared pointer.
///
class property_tree final : public std::enable_shared_from_this<property_tree>
{
public: // Typedefs
  using sptr_type = std::shared_ptr<property_tree>;
  using tree_type = property_tree_type;
  using path_type = property_path_type;

public: // Constants
  static constexpr std::string_view log_channel = "property_tree";

public: // Creator
  ///
  /// Property tree creator
  /// \param tree_file_path Tree file path
  /// \return
  ///
  [[nodiscard]] static auto create(const std::filesystem::path& tree_file_path) -> property_tree::sptr_type;

public: // Structors
  property_tree() = default;
  property_tree(const std::filesystem::path& tree_file_path);
  ~property_tree() noexcept;

public: // Modifiers
  ///
  /// Create a property in a property tree.
  /// If the property tree already has a value, the value of the created property will be the existing value.
  /// If a new value is created, the property and the tree value will be initialized with the `default_value`.
  /// \param path Path of property in tree
  /// \param default_value Default value of the property
  /// \return the newly created property
  ///
  template<typename T>
  [[nodiscard]] auto create_property(const property_path_type& path, T&& default_value) -> property<T>;

private: // Variables
  inline static std::mutex trees_mtx_{};
  inline static std::vector<std::weak_ptr<property_tree>> trees_{};
  mutable std::mutex mtx_;
  std::filesystem::path tree_file_path_;
  property_tree_type tree_;
};

///
///
template<typename T>
auto property_tree::create_property(const property_path_type& path, T&& default_value) -> property<T>
{
  const auto lock = std::lock_guard(mtx_);
  if(path.empty())
  {
    THROW_EXCEPTION(exception("register property failed: empty path"));
  }
  auto prop = property<T>(property_parser::read<T>(path, tree_).value_or(std::forward<decltype(default_value)>(default_value)));
  prop.property_tree_update_ = [sptr = shared_from_this(), path](const T& value)
  { property_parser::write(path, sptr->tree_, value); };
  prop.property_tree_update_(prop.value());
  return prop;
}

} // namespace bibstd::util
