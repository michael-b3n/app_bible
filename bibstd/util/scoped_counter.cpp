#include "util/scoped_counter.hpp"

namespace bibstd::util
{

///
///
scoped_incrementor::scoped_incrementor()
  : parent_{nullptr}
{
}

///
///
scoped_incrementor::scoped_incrementor(non_owning_ptr<scoped_counter> parent)
  : parent_{parent}
{
  if(parent_)
  {
    parent_->counter_.fetch_add(1);
  }
}

///
///
scoped_incrementor::~scoped_incrementor() noexcept
{
  if(parent_)
  {
    parent_->counter_.fetch_sub(1);
  }
}

///
///
scoped_incrementor::scoped_incrementor(const scoped_incrementor& other)
  : parent_{other.parent_}
{
  if(parent_)
  {
    parent_->counter_.fetch_add(1);
  }
}

///
///
scoped_incrementor::scoped_incrementor(scoped_incrementor&& other) noexcept
  : parent_{other.parent_}
{
  other.parent_ = nullptr;
}

///
///
auto scoped_incrementor::operator=(const scoped_incrementor& other) & -> scoped_incrementor&
{
  parent_ = other.parent_;
  if(parent_)
  {
    parent_->counter_.fetch_add(1);
  }
  return *this;
}

///
///
auto scoped_incrementor::operator=(scoped_incrementor&& other) & noexcept -> scoped_incrementor&
{
  if(parent_ && !other.parent_)
  {
    parent_->counter_.fetch_sub(1);
  }
  parent_ = other.parent_;
  other.parent_ = nullptr;
  return *this;
}

///
///
scoped_counter::scoped_counter()
  : counter_{0}
{
}

///
///
auto scoped_counter::count() const -> std::size_t
{
  return counter_.load();
}

///
///
auto scoped_counter::scoped_increment() -> scoped_incrementor
{
  return scoped_incrementor(this);
}

} // namespace bibstd::util
