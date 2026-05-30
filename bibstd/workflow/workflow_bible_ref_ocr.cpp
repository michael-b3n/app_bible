#include "bibstd/workflow/workflow_bible_ref_ocr.hpp"
#include "bibstd/bible/reference_range.hpp"
#include "bibstd/core/core_bible_ref_finder.hpp"
#include "bibstd/core/core_bible_ref_ocr.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/format.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/workflow/workflow_scripture.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

#include <ranges>
#include <string>

namespace bibstd::workflow
{

///
///
workflow_bible_ref_ocr_settings::workflow_bible_ref_ocr_settings()
  : tessdata_path{
      workflow_settings_->create_setting("ocr.tessdata_path", core::core_tesseract_common::tessdata_folder_finder())
    }
  , language{workflow_settings_->create_setting("ocr.language", util::language::german)}
  , recognize_largest_bounding_box{workflow_settings_->create_setting("ocr.recognize_largest_bounding_box", false)}
  , fallback_versification_name{workflow_settings_->create_setting(
      "ocr.fallback_versification_name",
      std::string{bible::scripture::versification_type::default_esv::name},
      std::make_shared<framework::setting_validator_list<setting_value_t<decltype(fallback_versification_name)>>>(
        workflow_scripture::default_versifications | std::views::keys |
        std::views::transform([](const auto& n) { return std::string{n}; }) | std::ranges::to<std::vector>()
      )
    )}
{
  // Check if the default value set above is contained in all default versification names.
  // If not the fallback versification name would be invalid.
  static_assert(meta::contains_v<
                bible::scripture::versification_type::all_defaults_variant,
                bible::scripture::versification_type::default_esv>);
}

///
///
workflow_bible_ref_ocr::workflow_bible_ref_ocr(std::shared_ptr<workflow_scripture> workflow_scripture)
  : core_bible_ref_finder_{std::make_unique<core::core_bible_ref_finder>()}
  , workflow_scripture_{std::move(workflow_scripture)}
{
  if(!settings->tessdata_path->value() || !std::filesystem::exists(*settings->tessdata_path->value()))
  {
    if(const auto found_tessdata_folder = core::core_tesseract_common::tessdata_folder_finder())
    {
      LOG_WARN("tessdata path setting invalid: used_alternative_folder=\"{}\"", found_tessdata_folder->generic_string());
      settings->tessdata_path->value(*found_tessdata_folder);
    }
  }
  const auto path = settings->tessdata_path->value();
  if(path && std::filesystem::exists(*path))
  {
    core_bible_ref_ocr_ = std::make_shared<core::core_bible_ref_ocr>(*path, settings->language->value());
  }
}

///
///
workflow_bible_ref_ocr::~workflow_bible_ref_ocr() noexcept = default;

///
///
auto workflow_bible_ref_ocr::find(const params& params) -> result
{
  try
  {
    const auto lock = std::scoped_lock{mtx_};
    if(!core_bible_ref_ocr_)
    {
      LOG_WARN("failed to start bible reference ocr search: not fully initialized");
      return return_failure;
    }
    // Capture screen directly on call of this function to ensure the cursor position is up-to-date.
    // This is usually called from the main thread and takes only a few milliseconds.
    // This also ensures that no displayed windows are blocking the screen capture.
    auto image_data = core_bible_ref_ocr_->capture_screen(params->cursor_position);
    if(!image_data)
    {
      LOG_WARN("capture screen failed: cursor_position={}", params->cursor_position);
      return return_failure;
    }
    LOG_INFO("find references: cursor_position={}", params->cursor_position);

    const auto local_settings = settings_local{
      .language = settings->language->value(),
      .recognize_largest_bounding_box = settings->recognize_largest_bounding_box->value(),
      .versification = versification()
    };
    const auto references = find_references(std::move(*image_data), local_settings);
    LOG_INFO("reference search finished: references=[{}]", util::format::join(references.value_or({}), ", "));

    if(references.has_value())
    {
      auto retval = result{};
      if(!references->empty())
      {
        const auto reference =
          std::ranges::min_element(*references, [](const auto& a, const auto& b) { return a.begin() < b.begin(); })->begin();
        const auto passage_params = workflow_scripture::passage_params::value_type{reference, std::nullopt};
        if(const auto passage_result = workflow_scripture_->passage(passage_params))
        {
          retval = result::value_type{
            .reference_ranges = *references, .first_reference = reference, .passage = passage_result.value().passage
          };
        }
      }
      return retval;
    }
    else
    {
      return return_failure;
    }
  }
  catch(...)
  {
    LOG_ERROR("exception occurred: {}", util::exception_report());
    return return_failure;
  }
}

///
///
auto workflow_bible_ref_ocr::versification() const -> versification_wrapper_type
{
  const auto scripture_result = workflow_scripture_->scripture(workflow_scripture::scripture_params::value_type{});
  if(scripture_result)
  {
    return versification_wrapper_type{scripture_result.value().scripture};
  }
  else
  {
    LOG_WARN("failed to get versification from selected default scripture: using fallback versification");
    return versification_wrapper_type{bible::scripture::versification_type{
      workflow_scripture::default_versifications.at(settings->fallback_versification_name->value())
    }};
  }
}

///
///
auto workflow_bible_ref_ocr::find_references(auto&& image_data, const settings_local& local_settings)
  -> framework::process_result<std::vector<bible::reference_range>>
{
  const auto relative_cursor_pos = image_data.relative_cursor_pos;
  const auto bounding_boxes = core_bible_ref_ocr_->recognize_bounding_box(std::move(image_data));
  if(!bounding_boxes)
  {
    LOG_DEBUG("missing bounding box: relative_cursor_pos={}", relative_cursor_pos);
    return {};
  }
  if(!core_bible_ref_ocr_->recognize_capture_area(*bounding_boxes, local_settings.recognize_largest_bounding_box))
  {
    LOG_WARN(
      "recognize capture area failed: bounding_box={}",
      local_settings.recognize_largest_bounding_box ? bounding_boxes->largest : bounding_boxes->reduced
    );
    return return_failure;
  }
  return parse_recognition(relative_cursor_pos, local_settings);
}

///
///
auto workflow_bible_ref_ocr::parse_recognition(
  const util::screen_coordinates_type& relative_cursor_pos, const settings_local& local_settings
) -> std::vector<bible::reference_range>
{
  auto references = std::vector<bible::reference_range>{};
  const auto position_data = core_bible_ref_ocr_->find_main_reference_position_data(relative_cursor_pos);
  if(position_data)
  {
    decltype(auto) versification = local_settings.versification.get();

    auto parse_result = core_bible_ref_finder_->parse(
      position_data->text, position_data->cursor_character_index, local_settings.language, versification
    );
    // If no references are found, we parse other high confidence OCR choices.
    // If a parse result is found and the area is valid we break out.
    if(parse_result.ranges.empty())
    {
      const auto position_data_choices = core_bible_ref_ocr_->find_reference_position_data_from_choices(relative_cursor_pos);
      std::ignore = std::ranges::any_of(
        position_data_choices,
        [&](const auto& position_data_choice)
        {
          parse_result = core_bible_ref_finder_->parse(
            position_data_choice.text, position_data_choice.cursor_character_index, local_settings.language, versification
          );
          return !parse_result.ranges.empty();
        }
      );
    }
    references = std::move(parse_result.ranges);
  }
  return references;
}

} // namespace bibstd::workflow
