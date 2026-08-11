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
/// Get the name of a book as provided by the default scripture, in the language of that scripture.
/// Falls back to the raw book identifier if the scripture does not provide a name.
/// \return book name
///
[[nodiscard]] auto bookName(bibstd::workflow::workflow_scripture& workflowScripture, bibstd::bible::book_id book) -> QString;

///
/// Get the copyright statement of the default scripture.
/// \return copyright statement, empty if the scripture does not provide one
///
[[nodiscard]] auto scriptureCopyright(bibstd::workflow::workflow_scripture& workflowScripture) -> QString;

///
/// Create a bible reference from QML provided values. The reference is validated
/// against the versification of the default scripture.
/// \return reference, or std::nullopt if the values do not describe a valid reference
///
[[nodiscard]] auto
toReference(bibstd::workflow::workflow_scripture& workflowScripture, const QString& bookId, int chapter, int verse)
  -> std::optional<bibstd::bible::reference>;

} // namespace bibqml
