#include "bibstd/bible/scripture.hpp"
#include <optional>

namespace bibstd::bible
{

///
///
scripture::scripture()
{
}

///
///
scripture::~scripture() noexcept = default;

///
///
auto scripture::information() const -> info_type
{
  return do_information();
}

///
///
auto scripture::passage_html(const reference_type& ref) const -> std::optional<passage_html_type>
{
  return do_passage_html(ref);
}

///
///
auto scripture::versification() const -> const versification_type&
{
  return do_versification();
}

} // namespace bibstd::bible
