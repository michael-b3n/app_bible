#pragma once

#include "bibstd/framework/process_params.hpp"

#include <expected>

namespace bibstd::workflow
{

///
/// Base class for workflows.
///
struct workflow_ground
{
  virtual ~workflow_ground() noexcept = default;
};

///
/// template specialization \see workflow_base
///
template<typename W>
class workflow_base : public workflow_ground
{
public: // Constants
  static constexpr std::unexpected return_failure{framework::process_result_unexpected::failure};
  static constexpr std::unexpected return_stopped{framework::process_result_unexpected::stopped};

protected: // Destructor
  virtual ~workflow_base() noexcept = default;
};

} // namespace bibstd::workflow
