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
  std::ranges::for_each(pool_, [](auto& element) { element = std::make_unique<active_worker>(); });
  return util::scoped_guard([]() { std::ranges::for_each(pool_, [](auto& element) { element.reset(); }); });
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
    pool_ids_, [&](const auto& ids) { return util::contains(ids, [id](const auto& p) { return p.strand_id == id; }); }
  );
  const auto worker_index = std::ranges::distance(std::ranges::cbegin(pool_ids_), dest_thread);
  if(worker_index < max_thread_count)
  {
    const auto queue_task = [&]
    { queue_task_index(task_data{std::move(task), task_id_type::new_uid(), id, rule}, worker_index); };
    switch(rule)
    {
    case queue_rule::append: queue_task(); break;
    case queue_rule::overwrite:
    {
      std::erase_if(pool_ids_.at(worker_index), [&](const auto& p) { return p.strand_id == id; });
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
  pool_ids_.at(index).emplace_back(id_pair{data.task_id, data.strand_id});
  const auto& worker = pool_.at(index);
  if(!worker)
  {
    THROW_EXCEPTION(std::runtime_error("invalid worker"));
  }
  worker->queue_task(create_task_wrapper(std::move(data), index));
}

///
///
auto thread_pool::queue_task_auto(task_data&& data) -> void
{
  const auto iter = std::ranges::min_element(pool_ids_, [&](const auto& a, const auto& b) { return a.size() < b.size(); });
  static_assert(pool_ids_.size() != 0);
  const auto index = std::ranges::distance(std::ranges::cbegin(pool_ids_), iter);
  pool_ids_.at(index).emplace_back(id_pair{data.task_id, data.strand_id});
  const auto& worker = pool_.at(index);
  if(!worker)
  {
    THROW_EXCEPTION(std::runtime_error("invalid worker"));
  }
  worker->queue_task(create_task_wrapper(std::move(data), index));
}

///
///
auto thread_pool::create_task_wrapper(task_data&& data, const std::size_t index) -> task_type
{
  return [forwarded_data = std::move(data), index]() mutable
  {
    auto lock = std::unique_lock(mtx_);
    const auto erase_count =
      std::erase_if(pool_ids_.at(index), [&](const auto& p) { return p.task_id == forwarded_data.task_id; });
    lock.unlock();
    if(erase_count != 0 && forwarded_data.task)
    {
      forwarded_data.task();
    }
  };
}

} // namespace bibstd::app_framework
