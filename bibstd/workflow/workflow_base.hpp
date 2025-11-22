#pragma once

#include "bibstd/framework/runtime_uid.hpp"
#include "bibstd/util/non_owning_ptr.hpp"
#include "bibstd/util/uid.hpp"

#include <expected>
#include <stop_token>

namespace bibstd::workflow
{

template<typename T>
concept has_auto_start = requires(T t) {
  { t.start() } -> std::same_as<std::stop_source>;
};

///
/// Common definitions and functions used by all workflows.
///
template<typename W, typename StartParamsType = void, typename ExpectedResultType = void>
class workflow_base
{
protected: // Typedefs
  ///
  /// Empty start parameters as default impl type for start params.
  ///
  struct empty_start_params final
  {};

  ///
  /// Generic start parameters for workflows.
  ///
  class start_params final
  {
  public: // Constructor
    start_params(const StartParamsType& params)
      requires(!std::is_void_v<StartParamsType>);
    start_params(StartParamsType&& params)
      requires(!std::is_void_v<StartParamsType>);
    start_params()
      requires(std::is_void_v<StartParamsType>)
    = default;

  public: // Accessors
    ///
    /// Access process ID.
    /// \return process ID
    ///
    [[nodiscard]] auto process_id() const -> framework::runtime_uid_type;

    ///
    /// Access start parameters.
    /// \return start parameters
    ///
    [[nodiscard]] auto operator->() const -> util::non_owning_ptr<const StartParamsType>
      requires(!std::is_void_v<StartParamsType>);

  private: // Variables
    framework::runtime_uid_type process_id_{};
    StartParamsType params_;
  };

  ///
  /// Unexpected result type for workflows.
  ///
  enum class unexpected_result
  {
    failure,
    stopped
  };

  ///
  /// Default result parameters for workflows containing process ID and result of result type.
  ///
  struct result_type final
  {
    framework::runtime_uid_type process_id{};
    std::expected<ExpectedResultType, unexpected_result> result{std::unexpected{unexpected_result::failure}};
  };

protected: // Constants
  static constexpr std::unexpected return_failure{unexpected_result::failure};
  static constexpr std::unexpected return_stopped{unexpected_result::stopped};

protected: // Destructor
  virtual ~workflow_base() noexcept = default;
};

///
///
template<typename W, typename StartParamsType, typename ExpectedResultType>
workflow_base<W, StartParamsType, ExpectedResultType>::start_params::start_params(const StartParamsType& params)
  requires(!std::is_void_v<StartParamsType>)
  : params_(params)
{
}

///
///
template<typename W, typename StartParamsType, typename ExpectedResultType>
workflow_base<W, StartParamsType, ExpectedResultType>::start_params::start_params(StartParamsType&& params)
  requires(!std::is_void_v<StartParamsType>)
  : params_(std::forward<decltype(params)>(params))
{
}

///
///
template<typename W, typename StartParamsType, typename ExpectedResultType>
auto workflow_base<W, StartParamsType, ExpectedResultType>::start_params::process_id() const -> framework::runtime_uid_type
{
  return process_id_;
}

///
///
template<typename W, typename StartParamsType, typename ExpectedResultType>
auto workflow_base<W, StartParamsType, ExpectedResultType>::start_params::operator->() const
  -> util::non_owning_ptr<const StartParamsType>
  requires(!std::is_void_v<StartParamsType>)
{
  return &params_;
}

} // namespace bibstd::workflow
