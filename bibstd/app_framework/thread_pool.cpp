#include "app_framework/thread_pool.hpp"
#include "util/contains.hpp"
#include "util/exception.hpp"
#include "util/log.hpp"

#include <algorithm>
#include <ranges>

namespace bibstd::app_framework
{

///
///
auto thread_pool::strand_id() -> strand_id_type
{
  return strand_id_type::new_uid();
}

///
///
auto thread_pool::init() -> util::scoped_guard
{
  const auto lock = std::lock_guard(mtx_);
  pool_.emplace_back(std::make_unique<pool_element>());
  return util::scoped_guard(
    []()
    {
      const auto lock = std::lock_guard(mtx_);
      pool_.clear();
    }
  );
}

///
///
auto thread_pool::queue_task(task_type&& task) -> void
{
  const auto lock = std::lock_guard(mtx_);
  static const auto internal_strand_id = strand_id();
  queue_task_auto(task_data{std::move(task), task_id_type::new_uid(), internal_strand_id, queue_rule::append});
}

///
///
auto thread_pool::queue_task(task_type&& task, const strand_id_type id, const queue_rule rule) -> void
{
  const auto lock = std::lock_guard(mtx_);
  const auto dest_thread = std::ranges::find_if(
    pool_, [&](const auto& e) { return util::contains(e->ids, [id](const auto& p) { return p.strand_id == id; }); }
  );
  const auto index = static_cast<std::size_t>(std::ranges::distance(std::ranges::cbegin(pool_), dest_thread));
  if(index < pool_.size())
  {
    const auto queue_task = [&] { queue_task_index(task_data{std::move(task), task_id_type::new_uid(), id, rule}, index); };
    switch(rule)
    {
    case queue_rule::append: queue_task(); break;
    case queue_rule::overwrite:
    {
      std::erase_if(pool_.at(index)->ids, [&](const auto& p) { return p.strand_id == id; });
      queue_task();
      break;
    }
    case queue_rule::discard: /*noop*/ break;
    }
  }
  else
  {
    queue_task_auto(task_data{std::move(task), task_id_type::new_uid(), id, rule});
  }
}

///
///
auto thread_pool::queue_task_index(task_data&& data, const std::size_t index) -> void
{
  decltype(auto) element = pool_.at(index);
  element->ids.emplace_back(id_pair{data.task_id, data.strand_id});
  element->worker.queue_task(create_task_wrapper(std::move(data), element.get()));
  remove_abandoned_workers();
}

///
///
auto thread_pool::queue_task_auto(task_data&& data) -> void
{
  const auto iter = std::ranges::find_if(pool_, [&](const auto& e) { return e->ids.empty(); });
  auto index = static_cast<std::size_t>(std::ranges::distance(std::ranges::cbegin(pool_), iter));
  if(iter == std::ranges::cend(pool_))
  {
    pool_.emplace_back(std::make_unique<pool_element>());
    index = pool_.size() - 1;
  }
  decltype(auto) element = pool_.at(index);
  element->ids.emplace_back(id_pair{data.task_id, data.strand_id});
  element->worker.queue_task(create_task_wrapper(std::move(data), element.get()));
  remove_abandoned_workers();
}

///
///
auto thread_pool::create_task_wrapper(task_data&& data, const util::non_owning_ptr<pool_element> element) -> task_type
{
  element->last_use = std::chrono::system_clock::now();
  return [forwarded_data = std::move(data), element]() mutable
  {
    auto pre_lock = std::unique_lock(mtx_);
    const auto found = util::contains(element->ids, [&](const auto& p) { return p.task_id == forwarded_data.task_id; });
    pre_lock.unlock();
    if(found && forwarded_data.task)
    {
      forwarded_data.task();
    }
    const auto post_lock = std::lock_guard(mtx_);
    std::erase_if(element->ids, [&](const auto& p) { return p.task_id == forwarded_data.task_id; });
  };
}

///
///
auto thread_pool::remove_abandoned_workers() -> void
{
  using ms = std::chrono::milliseconds;
  const auto now = std::chrono::system_clock::now();
  std::erase_if(
    pool_,
    [&](const auto& element)
    {
      const auto empty_ids = element->ids.empty();
      const auto inactive_duration = now > element->last_use ? std::chrono::duration_cast<ms>(now - element->last_use) : ms{0};
      return empty_ids && inactive_duration > std::chrono::minutes{1};
    }
  );
}

} // namespace bibstd::app_framework
