#pragma once

#include "bibstd/bible/scripture.hpp"
#include "bibstd/framework/process_params.hpp"
#include "bibstd/framework/settings_base.hpp"
#include "bibstd/util/const_map.hpp"
#include "bibstd/workflow/workflow_base.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

#include <filesystem>
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
  ~workflow_scripture_settings() noexcept override = default;

public: // Constants
  static constexpr auto default_folder_name = "scriptures";

public: // Variables
  const setting_type<std::optional<std::string>> scripture_name;
  const setting_type<std::filesystem::path> scripture_folder;
  const setting_type<std::string> fallback_versification;
};

///
/// Workflow for scripture. The scriptures are the zip files in the folder of the setting "scripture.folder",
/// by default the folder "scriptures" in the local data folder. They are loaded on construction.
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
    explicit versification_wrapper(bible::scripture::versification_type versification);

  public: // Accessors
    ///
    /// Get the underlying versification.
    /// \return the underlying versification
    ///
    auto get() const -> const bible::scripture::versification_type&;

  private: // Variables
    std::variant<bible::scripture::versification_type, std::shared_ptr<bible::scripture>> data_;
  };

  struct scripture_params_t final
  {
    std::optional<std::string> scripture_name;
  };

  struct scripture_result_t final
  {
    std::string name;
    std::shared_ptr<bible::scripture> scripture;
  };

  struct passage_params_t final
  {
    bible::scripture::reference_type reference;
    std::optional<std::string> scripture_name;
  };

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
  ~workflow_scripture() noexcept override;

public: // Accessors
  ///
  /// Get scripture. If no scripture name is provided in the params,
  /// the scripture defined in the settings will be used.
  /// \return scripture, or an unexpected result in case of failure
  ///
  [[nodiscard]] auto scripture(const scripture_params& params) const -> scripture_result;

  ///
  /// Get the versification of the specifieds scripture, or the fallback versification
  /// if the scripture is not available. No matter if there are no scriptures loaded or
  /// the scripture name does not exist, the fallback versification is always returned.
  /// \return versification
  ///
  [[nodiscard]] auto versification_or_fallback(const scripture_params& params) const -> versification_wrapper_type;

  ///
  /// Get passage from scripture. If no scripture name is provided in the params,
  /// the scripture defined in the settings will be used.
  /// \return passage, or an unexpected result in case of failure
  ///
  [[nodiscard]] auto passage(const passage_params& params) const -> passage_result;

private: // Implementation
  auto init() -> void;
};

} // namespace bibstd::workflow
