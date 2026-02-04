#pragma once

#include <string_view>

namespace bibstd::util
{

///
/// Language enumeration.
///
enum class language
{
  english,
  german,
};

///
/// Get pretty name of language.
/// \param lang Language enum value
/// \return Pretty name of language
///
constexpr auto pretty_name(const language lang) -> std::string_view
{
  switch(lang)
  {
  case language::english: return "English";
  case language::german: return "Deutsch";
  default: return "Unknown";
  }
}

} // namespace bibstd::util
