#include "bibqml/util/ScriptureAccess.hpp"

#include <bibstd/bible/common.hpp>
#include <bibstd/bible/scripture.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

#include <algorithm>
#include <array>

namespace bibqml
{

///
///
auto defaultScripture(bibstd::workflow::workflow_scripture& workflowScripture)
  -> std::optional<std::shared_ptr<bibstd::bible::scripture>>
{
  static constexpr auto defaultScriptureParams = bibstd::workflow::workflow_scripture::scripture_params::value_type{};
  auto scripture = workflowScripture.scripture(defaultScriptureParams);
  if(!scripture)
  {
    LOG_WARN("failed to get default scripture");
    return std::nullopt;
  }
  return scripture.value().scripture;
}

///
///
auto bookName(bibstd::workflow::workflow_scripture& workflowScripture, const bibstd::bible::book_id book) -> QString
{
  static constexpr auto identifier = [](const bibstd::bible::book_id id)
  {
    const auto name = bibstd::util::enum_name(id);
    return QString::fromLatin1(name.data(), static_cast<qsizetype>(name.size()));
  };

  const auto scripture = defaultScripture(workflowScripture);
  if(!scripture)
  {
    return identifier(book);
  }
  const auto names = scripture.value()->book_information(book);
  if(!names)
  {
    return identifier(book);
  }
  // the short name is the form meant for display, the others only serve as fallbacks
  const auto candidates = std::array{&names->short_name, &names->abbreviation, &names->long_name};
  const auto* const found = std::ranges::find_if(candidates, [](const auto* name) { return !name->empty(); });
  return found != std::ranges::cend(candidates) ? QString::fromStdString(**found) : identifier(book);
}

///
///
auto scriptureCopyright(bibstd::workflow::workflow_scripture& workflowScripture) -> QString
{
  const auto scripture = defaultScripture(workflowScripture);
  if(!scripture)
  {
    return {};
  }
  const auto copyright = scripture.value()->information().copyright;
  return copyright ? QString::fromStdString(*copyright) : QString{};
}

///
///
auto toReference(
  bibstd::workflow::workflow_scripture& workflowScripture, const QString& bookId, const int chapter, const int verse
) -> std::optional<bibstd::bible::reference>
{
  const auto book = bibstd::util::to_enum<bibstd::bible::book_id>(bookId.toStdString());
  if(!book)
  {
    LOG_WARN("invalid book id: {}", bookId.toStdString());
    return std::nullopt;
  }
  const auto scripture = defaultScripture(workflowScripture);
  if(!scripture)
  {
    return std::nullopt;
  }
  const auto ref = bibstd::bible::reference::create(*book, chapter, verse, scripture.value()->versification());
  if(!ref)
  {
    LOG_WARN("invalid reference: {} {}, {}", bookId.toStdString(), chapter, verse);
    return std::nullopt;
  }
  return ref;
}

} // namespace bibqml
