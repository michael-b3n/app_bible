#include "bibstd/bible/parser.hpp"

namespace bibstd::bible
{

///
///
parser::parser()
{
}

///
///
parser::~parser() noexcept = default;

///
///
auto parser::valid() const -> bool
{
  return do_valid();
}

///
///
auto parser::info() const -> scripture_info
{
  return do_info();
}

///
///
auto parser::passage_html(const reference& ref) const -> std::expected<html_passage, error_code>
{
  return do_passage_html(ref);
}

} // namespace bibstd::bible
