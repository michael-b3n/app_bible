#pragma once

#include "bibstd/bible/scripture.hpp"
#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/workflow/workflow_base.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

#include <memory>
#include <mutex>
#include <optional>
#include <variant>

// Forward declarations
namespace bibstd::core
{
class core_scripture_store;
} // namespace bibstd::core

namespace bibstd::workflow
{

///
/// Settings corresponding to workflow scripture.
///
class workflow_scripture_settings final : public framework::settings_base
{
public: // Structors
  workflow_scripture_settings(std::shared_ptr<workflow_settings> workflow_settings);
  ~workflow_scripture_settings() noexcept = default;

public: // Variables
  const setting_type<std::optional<std::string>> scripture_name;
};

///
/// Workflow for scripture.
///
class workflow_scripture final : public workflow_base<workflow_scripture_settings>
{
  // Typedefs
  ///
  /// Wrapper for bible scripture versification.
  /// This wrapper is needed to provide access to the versification
  /// of a scripture without exposing the whole scripture.
  ///
  class versification_wrapper final
  {
  public: // Constructor
    explicit versification_wrapper(std::shared_ptr<bible::scripture> scripture);
    explicit versification_wrapper(bible::scripture::versification_type&& versification);

  public: // Accessors
    ///
    /// Get the underlying versification.
    /// \return the underlying versification
    ///
    auto get() const -> const bible::scripture::versification_type&;

  private: // Variables
    std::variant<bible::scripture::versification_type, std::shared_ptr<bible::scripture>> data_;
  };

  ///
  /// Struct containing params for scripture accessor.
  ///
  struct scripture_params_t final
  {
    std::optional<std::string> scripture_name;
  };

  ///
  /// Struct containing result for scripture accessor.
  ///
  struct scripture_result_t final
  {
    std::string name;
    std::shared_ptr<bible::scripture> scripture;
  };

  ///
  /// Struct containing params for passage accessor.
  ///
  struct passage_params_t final
  {
    bible::scripture::reference_type reference;
    std::optional<std::string> scripture_name;
  };

  ///
  /// Struct containing result for passage accessor.
  ///
  struct passage_result_t final
  {
    bible::scripture::passage_html_type passage;
  };

  // Variables
  mutable std::mutex mtx_;
  const std::unique_ptr<core::core_scripture_store> core_scripture_store_;

public: // Constants
  static constexpr auto default_versifications = []()
  {
    using all_defaults_variant = bible::scripture::versification_type::all_defaults_variant;
    return [&]<std::size_t... I>(std::index_sequence<I...>)
    {
      return util::make_const_bimap<std::string_view, bible::scripture::versification_type>({
        {std::string_view{meta::pack_info<all_defaults_variant>::type_at<I>::name},
         bible::scripture::versification_type{meta::pack_info<all_defaults_variant>::type_at<I>{}}}
        ...
      });
    }(std::make_index_sequence<meta::pack_info<all_defaults_variant>::size>{});
  }();

public: // Typedefs
  using versification_wrapper_type = versification_wrapper;
  using scripture_params = framework::process_params<scripture_params_t>;
  using scripture_result = framework::process_result<scripture_result_t>;
  using passage_params = framework::process_params<passage_params_t>;
  using passage_result = framework::process_result<passage_result_t>;

public: // Structors
  workflow_scripture(std::shared_ptr<workflow_settings> workflow_settings);
  ~workflow_scripture() noexcept;

public: // Accessors
  ///
  /// Get scripture. If no scripture name is provided in the params,
  /// the scripture defined in the settings will be used.
  /// \param params Process parameters containing an optional scripture name
  /// \return scripture, or an unexpected result in case of failure
  ///
  [[nodiscard]] auto scripture(const scripture_params& params) -> scripture_result;

  ///
  /// Get passage from scripture. If no scripture name is provided in the params,
  /// the scripture defined in the settings will be used.
  /// \param params Process parameters containing the reference and optional scripture name
  /// \return passage, or an unexpected result in case of failure
  ///
  [[nodiscard]] auto passage(const passage_params& params) -> passage_result;

private: // Implementation
  auto init() -> void;
};

} // namespace bibstd::workflow
