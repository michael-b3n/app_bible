#include "core/core_bible_reference_ocr.hpp"
#include "bible/book_name_variants_de.hpp"
#include "util/log.hpp"
#include "util/string.hpp"

#include <algorithm>

namespace bibstd::core
{

///
///
auto core_bible_reference_ocr::match_choices_to_bible_book(
  const std::vector<tesseract_choices>& choices_list,
  const std::function<bool(const tesseract_choices&)>& choices_filter
) const -> std::vector<txt::indexed_strings>
{
  auto results = std::vector<txt::indexed_strings>{};
  std::ranges::for_each(
    bible::book_name_variants_de::name_variants_list,
    [&](const auto& element)
    {
      const auto& [_, name_variant] = element;
      const auto element_result = match_choices_to_string(choices_list, name_variant, choices_filter);
      results.insert(results.cend(), element_result.cbegin(), element_result.cend());
    }
  );
  return results;
}

///
///
auto core_bible_reference_ocr::match_choices_to_string(
  const std::vector<tesseract_choices>& choices_list,
  const std::string_view text_template,
  const std::function<bool(const tesseract_choices&)>& choices_filter
) const -> std::vector<txt::indexed_strings>
{
  auto results = std::vector<txt::indexed_strings>{};
  if(!text_template.empty())
  {
    const auto indexed_strings = [&]
    {
      auto result = txt::indexed_strings{};
      std::ranges::for_each(choices_list, [&](const auto& choices) mutable { result.append_string(choices.front().symbol); });
      return result;
    }();

    std::ranges::for_each(
      std::views::iota(decltype(choices_list.size()){0}, choices_list.size()),
      [&](const auto choices_list_offset) mutable
      {
        auto text_template_found = false;
        auto local_indexed_strings = indexed_strings;
        std::ranges::any_of(
          std::views::iota(choices_list_offset, choices_list.size()) |
            std::views::filter([&](const auto index) { return choices_filter(choices_list.at(index)); }),
          [&, text_template_position = std::size_t{0}](const auto choices_list_index) mutable
          {
            const auto& choices = choices_list.at(choices_list_index);
            const auto choice_optional = find_chars_begin_match(choices, text_template.substr(text_template_position));
            if(choice_optional)
            {
              text_template_position += choice_optional->symbol.size();
              local_indexed_strings.overwrite_at(choices_list_index, choice_optional->symbol);

              text_template_found = text_template_position >= text_template.size();
              if(text_template_found)
              {
                text_template_position = 0;
              }
              return text_template_found;
            }
            return true;
          }
        );
        if(text_template_found)
        {
          results.emplace_back(std::move(local_indexed_strings));
        }
      }
    );
  }
  return results;
}

///
///
auto core_bible_reference_ocr::find_chars_begin_match(const tesseract_choices& choices, const std::string_view chars) const
  -> std::optional<tesseract_choice>
{
  auto result = std::optional<tesseract_choice>{};
  std::ranges::for_each(
    choices,
    [&](const auto& choice)
    {
      auto match_found = util::starts_with(chars, choice.symbol);
      if(match_found)
      {
        result = choice;
      }
    }
  );
  return result;
}

} // namespace bibstd::core
