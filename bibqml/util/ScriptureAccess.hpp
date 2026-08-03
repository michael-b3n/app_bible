#pragma once

#include <bibstd/bible/reference.hpp>

#include <QString>

#include <memory>
#include <optional>

// Forward declarations
namespace bibstd::bible
{
class scripture;
} // namespace bibstd::bible
namespace bibstd::workflow
{
class workflow_scripture;
} // namespace bibstd::workflow

namespace bibqml
{

///
/// Get the default scripture from the workflow scripture.
/// \return default scripture, or std::nullopt if no scripture could be obtained
///
[[nodiscard]] auto defaultScripture(bibstd::workflow::workflow_scripture& workflowScripture)
  -> std::optional<std::shared_ptr<bibstd::bible::scripture>>;

///
/// Create a bible reference from QML provided values. The reference is validated
/// against the versification of the default scripture.
/// \return reference, or std::nullopt if the values do not describe a valid reference
///
[[nodiscard]] auto
toReference(bibstd::workflow::workflow_scripture& workflowScripture, const QString& bookId, int chapter, int verse)
  -> std::optional<bibstd::bible::reference>;

} // namespace bibqml
