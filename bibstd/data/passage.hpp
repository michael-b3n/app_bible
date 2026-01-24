#pragma once

#include "bibstd/bible/passage_info.hpp"

namespace bibstd::data
{

///
/// Struct containing data of a bible passage.
///
template<typename T>
struct passage final
{
  // Constructor
  passage(const bible::passage_info& info, std::string_view content, std::string_view licence);
  passage(const bible::passage_info& info, std::string&& content, std::string&& licence);

  // Operators
  auto operator==(const passage&) const -> bool = default;

  // Variables
  bible::passage_info info;
  std::string content;
  std::string licence;
};

///
/// HTML tagged passage specialization.
///
using passage_html = passage<struct html_tag>;

///
///
template<typename T>
passage<T>::passage(const bible::passage_info& info, const std::string_view content, const std::string_view licence)
  : info{info}
  , content{content}
  , licence{licence}
{
}

///
///
template<typename T>
passage<T>::passage(const bible::passage_info& info, std::string&& content, std::string&& licence)
  : info{info}
  , content{std::move(content)}
  , licence{std::move(licence)}
{
}

} // namespace bibstd::data
