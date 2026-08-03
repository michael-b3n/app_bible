#include "bibqml/util/ScriptureAccess.hpp"

#include <bibstd/bible/common.hpp>
#include <bibstd/bible/scripture.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/workflow/workflow_scripture.hpp>

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
