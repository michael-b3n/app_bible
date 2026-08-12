#include "bibstd/workflow/workflow_bible_ref_ocr.hpp"
#include "bibstd/bible/reference_ocr.hpp"
#include "bibstd/core/core_bible_ref_finder.hpp"
#include "bibstd/system/ocr.hpp"
#include "bibstd/txt/ocr_engine.hpp"
#include "bibstd/txt/ocr_engine_tesseract.hpp"
#include "bibstd/util/exception.hpp"
#include "bibstd/util/format.hpp"
#include "bibstd/util/log.hpp"
#include "bibstd/util/visit_helper.hpp"
#include "bibstd/workflow/workflow_scripture.hpp"
#include "bibstd/workflow/workflow_settings.hpp"

#include <memory>
#include <ranges>
#include <string>
#include <variant>

namespace bibstd::workflow
{
namespace detail
{

///
/// Compute the bounding box of the recognized reference within the image. The bounding box
/// surrounds the bounding boxes of all characters the reference was parsed from.
/// \return bounding box of the reference, or std::nullopt if no bounding box could be determined
///
[[nodiscard]] auto reference_bounding_box(
  const bible::reference_ocr::reference_position_data& position_data,
  const core::core_bible_ref_finder::index_range_type& index_range
) -> std::optional<util::screen_rect_type>
{
  decltype(auto) boxes = position_data.character_bounding_boxes;
  const auto begin = std::ranges::next(std::ranges::cbegin(boxes), std::min(index_range.begin, boxes.size()));
  const auto end = std::ranges::next(std::ranges::cbegin(boxes), std::min(index_range.end, boxes.size()));

  auto result = std::optional<util::screen_rect_type>{};
  std::ranges::for_each(
    std::ranges::subrange(begin, end) | std::views::filter([](const auto& box) { return box.has_value(); }),
    [&](const auto& box) { result = result ? math::surrounding_rect(*result, *box) : *box; }
  );
  return result;
}

} // namespace detail

///
///
workflow_bible_ref_ocr_settings::workflow_bible_ref_ocr_settings(std::shared_ptr<workflow_settings> workflow_settings)
  : framework::settings_base{std::move(workflow_settings)}
  , tessdata_path{workflow_settings_->create_setting("ocr.tessdata_path", txt::ocr_engine_tesseract::tessdata_folder_finder())}
  , character_recognition_ocr_engine{workflow_settings_->create_setting(
      "ocr.character_recognition_ocr_engine",
      setting_value_t<decltype(character_recognition_ocr_engine)>{},
      std::make_shared<framework::setting_validator_list<setting_value_t<decltype(character_recognition_ocr_engine)>>>()
    )}
  , layout_recognition_ocr_engine{workflow_settings_->create_setting(
      "ocr.layout_recognition_ocr_engine",
      setting_value_t<decltype(layout_recognition_ocr_engine)>{},
      std::make_shared<framework::setting_validator_list<setting_value_t<decltype(layout_recognition_ocr_engine)>>>()
    )}
  , language{workflow_settings_->create_setting("ocr.language", util::language::german)}
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
workflow_bible_ref_ocr::workflow_bible_ref_ocr(
  std::shared_ptr<workflow_settings> workflow_settings, std::shared_ptr<workflow_scripture> workflow_scripture
)
  : workflow_base{std::move(workflow_settings)}
  , core_bible_ref_finder_{std::make_unique<core::core_bible_ref_finder>()}
  , workflow_scripture_{std::move(workflow_scripture)}
{
  init();
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

    const auto character_recognition_ocr_engine = settings().character_recognition_ocr_engine->value();
    if(!character_recognition_ocr_engine.has_value() || ocr_engines_.empty())
    {
      LOG_WARN("failed to start bible reference ocr search: no ocr engines initialized");
      return return_failure;
    }
    const auto local_settings = settings_t{
      .character_recognition_ocr_engine = *character_recognition_ocr_engine,
      .layout_recognition_ocr_engine = settings().layout_recognition_ocr_engine->value(),
      .language = settings().language->value(),
      .versification = versification()
    };
    LOG_INFO(
      "find references: image=[width={}, height={}], position=[{}]",
      params->image.width(),
      params->image.height(),
      params->position
    );

    const auto construct_result = [&](const auto& found)
    {
      auto retval = result{};
      if(!found.ranges.empty())
      {
        auto ranges = found.ranges;
        std::ranges::sort(ranges, [](const auto& a, const auto& b) { return a.begin() < b.begin(); });
        retval = result::value_type{.reference_ranges = std::move(ranges), .reference_bounding_box = found.bounding_box};
      }
      return retval;
    };

    using atype = bible::reference_ocr::algorithm_type;
    if(
      const auto references = find_references(params, local_settings, atype::recognize_with_paragraph_recognition);
      references && !references->ranges.empty()
    )
    {
      LOG_INFO("reference search finished: references=[{}]", util::format::join(references->ranges, ", "));
      return construct_result(*references);
    }
    else if(const auto references = find_references(params, local_settings, atype::recognize_just_with_line_recognition))
    {
      LOG_INFO("reference search finished: references=[{}]", util::format::join(references->ranges, ", "));
      return construct_result(*references);
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
auto workflow_bible_ref_ocr::init() -> void
{
  const auto lock = std::scoped_lock{mtx_};
  auto ocr_engine_system = system::ocr::create(settings().language->value());
  if(!std::holds_alternative<std::monostate>(ocr_engine_system))
  {
    ocr_engines_.emplace_back(std::move(ocr_engine_system));
  }
  if(!settings().tessdata_path->value() || !std::filesystem::exists(*settings().tessdata_path->value()))
  {
    if(const auto found_tessdata_folder = txt::ocr_engine_tesseract::tessdata_folder_finder())
    {
      LOG_WARN("tessdata path setting invalid: used_alternative_folder=\"{}\"", found_tessdata_folder->generic_string());
      settings().tessdata_path->value(*found_tessdata_folder);
    }
  }
  const auto path = settings().tessdata_path->value();
  if(path && std::filesystem::exists(*path))
  {
    txt::ocr_engine_tesseract::uptr_type ocr_engine_tesseract =
      std::make_unique<txt::ocr_engine_tesseract>(*path, settings().language->value());
    ocr_engines_.emplace_back(std::move(ocr_engine_tesseract));
  }

  auto character_recognition_engines = std::vector<std::string>{};
  auto layout_recognition_engines = std::vector<std::string>{};
  std::ranges::for_each(
    ocr_engines_,
    [&](const auto& engine)
    {
      util::visit_lambdas(
        engine,
        []([[maybe_unused]] const std::monostate&) {},
        [&](const txt::ocr_engine<txt::ocr_engine_tag_plain>::uptr_type& e)
        { character_recognition_engines.emplace_back(e->name()); },
        [&](const txt::ocr_engine<txt::ocr_engine_tag_layout_analysis>::uptr_type& e)
        {
          character_recognition_engines.emplace_back(e->name());
          layout_recognition_engines.emplace_back(e->name());
        }
      );
    }
  );
  using validator_type = framework::setting_validator_list<std::optional<std::string>>::sptr_type;
  decltype(auto) cr_validator = std::get<validator_type>(settings().character_recognition_ocr_engine->validator);
  std::ignore = cr_validator->available(character_recognition_engines);
  if(!character_recognition_engines.empty())
  {
    settings().character_recognition_ocr_engine->value(character_recognition_engines.front());
  }
  decltype(auto) lr_validator = std::get<validator_type>(settings().layout_recognition_ocr_engine->validator);
  std::ignore = lr_validator->available(layout_recognition_engines);
  if(!layout_recognition_engines.empty())
  {
    settings().layout_recognition_ocr_engine->value(layout_recognition_engines.front());
  }
}

///
///
auto workflow_bible_ref_ocr::versification() const -> decltype(settings_t::versification)
{
  const auto scripture_result = workflow_scripture_->scripture(workflow_scripture::scripture_params::value_type{});
  if(scripture_result)
  {
    return decltype(settings_t::versification){scripture_result.value().scripture};
  }
  else
  {
    LOG_WARN("failed to get versification from selected default scripture: using fallback versification");
    return decltype(settings_t::versification){bible::scripture::versification_type{
      workflow_scripture::default_versifications.at(settings().fallback_versification_name->value())
    }};
  }
}

///
///
auto workflow_bible_ref_ocr::find_references(const auto& params, const settings_t& settings, const auto algorithm)
  -> framework::process_result<find_references_result_t>
{
  const auto position_data = bible::reference_ocr::run(
    ocr_engines_,
    params->image,
    params->position,
    bible::reference_ocr::algorithm_data{
      .algorithm = algorithm,
      .engine_name_character_recognition = settings.character_recognition_ocr_engine,
      .engine_name_layout_recognition = settings.layout_recognition_ocr_engine
    }
  );
  if(position_data)
  {
    decltype(auto) versification = settings.versification.get();

    auto parse_result = core_bible_ref_finder_->parse(
      position_data->text, position_data->cursor_character_index, settings.language, versification
    );
    return find_references_result_t{
      .ranges = std::move(parse_result.ranges),
      .bounding_box = detail::reference_bounding_box(*position_data, parse_result.index_range_origin)
    };
  }
  else
  {
    return return_failure;
  }
}

} // namespace bibstd::workflow
