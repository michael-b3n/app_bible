#pragma once

#include "bibstd/bible/passage_info.hpp"

namespace bibstd::bible
{

///
/// Struct containing data of a bible passage.
///
template<typename T>
struct passage final
{
  // Constructor
  passage(const bible::passage_info& info, std::string_view content);
  passage(const bible::passage_info& info, std::string&& content);

  // Operators
  auto operator==(const passage&) const -> bool = default;

  // Variables
  bible::passage_info info;
  std::string content;
};

///
/// HTML tagged passage specialization.
///
using passage_html = passage<struct html_tag>;

///
///
template<typename T>
passage<T>::passage(const bible::passage_info& info, const std::string_view content)
  : info{info}
{
}

///
///
template<typename T>
passage<T>::passage(const bible::passage_info& info, std::string&& content)
  : info{info}
  , content{std::move(content)}
{
}

} // namespace bibstd::bible
