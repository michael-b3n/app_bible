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
  return util::new_uid<strand_id_type::tag_type>();
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
auto thread_pool::queue_task(task_type&& task, const std::optional<strand_id_type> id) -> void
{
  const auto lock = std::lock_guard(mtx_);
  queue_task_impl(std::move(task), id);
}

///
///
auto thread_pool::queue_task(task_type&& task, const delay_type delay, const std::optional<strand_id_type> id) -> void
{
  const auto time_point = std::chrono::system_clock::now() + delay;
  queue_task(std::move(task), time_point, id);
}

///
///
auto thread_pool::queue_task(task_type&& task, const time_point_type time_point, const std::optional<strand_id_type> id) -> void
{
  if(time_point < std::chrono::system_clock::now())
  {
    queue_task(std::move(task), id);
  }
  else
  {
    const auto lock = std::lock_guard(mtx_);
    delayed_tasks_.emplace_back(delayed_task{.task_ = std::move(task), .strand_id_ = id, .run_time_ = time_point});
    if(std::ranges::all_of(pool_ids_, [](const auto& ids) { return ids.empty(); }))
    {
      handle_delayed_tasks();
    }
  }
}

///
///
auto thread_pool::queue_task_impl(task_type&& task, std::optional<strand_id_type> id) -> void
{
  if(id)
  {
    const auto dest_thread = std::ranges::find_if(pool_ids_, [&](const auto& ids) { return util::contains(ids, *id); });
    const auto worker_index = std::ranges::distance(std::ranges::cbegin(pool_ids_), dest_thread);
    if(worker_index < max_thread_count)
    {
      queue_task_index(std::move(task), *id, worker_index);
    }
    else
    {
      queue_task_auto(std::move(task), *id);
    }
  }
  else
  {
    static const auto internal_strand_id = strand_id();
    queue_task_auto(std::move(task), internal_strand_id);
  }
}

///
///
auto thread_pool::handle_delayed_tasks() -> void
{
  if(delayed_tasks_.empty())
  {
    return;
  }
  const auto now = std::chrono::system_clock::now();
  auto current_delay = delay_type::max();
  const auto removed = std::erase_if(
    delayed_tasks_,
    [&](auto& delayed_task)
    {
      const auto retval = delayed_task.run_time_ <= now;
      if(retval)
      {
        queue_task_impl(std::move(delayed_task.task_), delayed_task.strand_id_);
      }
      else
      {
        const auto delay_to_task = delayed_task.run_time_ - now;
        current_delay = std::min(current_delay, delay_to_task);
      }
      return retval;
    }
  );
  if(removed == 0 && std::ranges::all_of(pool_ids_, [](const auto& ids) { return ids.empty(); }))
  {
    const auto dead_time = std::min(current_delay, std::chrono::duration_cast<delay_type>(std::chrono::milliseconds(100)));
    queue_task_impl([dead_time]() { std::this_thread::sleep_for(dead_time); }, std::nullopt);
  }
}

///
///
auto thread_pool::queue_task_index(task_type&& task, const strand_id_type id, const std::size_t index) -> void
{
  pool_ids_.at(index).push_back(id);
  const auto& worker = pool_.at(index);
  if(!worker)
  {
    THROW_EXCEPTION(std::runtime_error("invalid worker"));
  }
  worker->queue_task(create_task_wrapper(std::move(task), index));
}

///
///
auto thread_pool::queue_task_auto(task_type&& task, const strand_id_type id) -> void
{
  const auto iter = std::ranges::min_element(pool_ids_, [&](const auto& a, const auto& b) { return a.size() < b.size(); });
  static_assert(pool_ids_.size() != 0);
  const auto index = std::ranges::distance(std::ranges::cbegin(pool_ids_), iter);
  pool_ids_.at(index).push_back(id);
  const auto& worker = pool_.at(index);
  if(!worker)
  {
    THROW_EXCEPTION(std::runtime_error("invalid worker"));
  }
  worker->queue_task(create_task_wrapper(std::move(task), index));
}

///
///
auto thread_pool::create_task_wrapper(task_type&& task, const std::size_t index) -> task_type
{
  return [forwarded_task = std::move(task), index]() mutable
  {
    forwarded_task();
    const auto lock = std::lock_guard(mtx_);
    pool_ids_.at(index).pop_front();
    handle_delayed_tasks();
  };
}

} // namespace bibstd::app_framework
