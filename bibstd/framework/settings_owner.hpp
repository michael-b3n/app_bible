#pragma once

#include <memory>
#include <type_traits>

namespace bibstd::framework
{

///
/// Settings owner.
///
template<typename T>
  requires(std::is_default_constructible_v<T>)
class settings_owner
{
public: // Typedefs
  using settings_type = std::shared_ptr<T>;

public: // Variables
  const settings_type settings{std::make_shared<T>()};
};

} // namespace bibstd::framework
