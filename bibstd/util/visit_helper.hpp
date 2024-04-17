#pragma once

#include <variant>

namespace bibstd::util
{
namespace detail
{

///
/// Overloaded helper type.
/// \see https://en.cppreference.com/w/cpp/utility/variant/visit
///
template<class... Ts>
struct overloaded : Ts...
{
  using Ts::operator()...;
};

template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace detail

///
/// Overload function for `std::visit` handling multiple lambdas.
/// \param var Variant that shall be visited
/// \param ...lambdas Lambdas that shall be called on visitors elements.
/// \return as `std::visit` would return
///
constexpr auto visit_lambdas(auto&& var, auto&&... lambdas) -> decltype(auto)
{
  return std::visit(detail::overloaded{std::forward<decltype(lambdas)>(lambdas)...}, std::forward<decltype(var)>(var));
}

} // namespace bibstd::util
