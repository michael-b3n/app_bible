#include "util/signals.hpp"

namespace util
{

///
///
connection_store::~connection_store() noexcept
{
  clear();
}

///
///
auto connection_store::add_connection(scoped_connection_type&& con) -> void
{
  const auto lock = std::scoped_lock(mtx_);
  connections_.emplace_back(std::move(con));
}

///
///
auto connection_store::clear() -> void
{
  const auto lock = std::scoped_lock(mtx_);
  connections_.clear();
}

} // namespace util
