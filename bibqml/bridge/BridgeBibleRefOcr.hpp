#pragma once

#include <bibstd/bible/reference_range.hpp>
#include <bibstd/framework/process_params.hpp>
#include <bibstd/framework/setting_type_erased.hpp>
#include <bibstd/signal/synchronized_executor.hpp>
#include <bibstd/util/non_owning_ptr.hpp>
#include <bibstd/util/screen_types.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>

#include <QObject>
#include <QPoint>
#include <QRect>
#include <qtmetamacros.h>
#include <QtQml/qqmlregistration.h>

#include <memory>
#include <optional>
#include <string>

namespace bibstd::workflow
{
// Forward declarations
class workflow_bible_ref_ocr;
class workflow_settings;
} // namespace bibstd::workflow

namespace bibqml
{

///
/// QML bridge for workflow_bible_ref_ocr.
/// Connects to workflow signals and exposes data via Q_PROPERTY.
///
class BridgeBibleRefOcr final : public QObject
{
  Q_OBJECT
  QML_ELEMENT

  Q_PROPERTY(bool running MEMBER running_ NOTIFY runningChanged)
  Q_PROPERTY(QPoint cursorPosition MEMBER cursorPosition_ NOTIFY cursorPositionChanged)

  // Typedefs
  using ClickActionSettingType = bibstd::util::non_owning_ptr<bibstd::framework::setting_type_erased<std::string>>;

  // Variables
  const std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr> workflowBibleRefOcr_;
  const bibstd::workflow::workflow_hotkey::shared_sig_type findReferenceSig_{};
  const ClickActionSettingType clickActionSetting_;
  bibstd::framework::process_id_type processId_{};
  bool running_{false};
  QPoint cursorPosition_{0, 0};
  bibstd::signal::synchronized_executor executor_;

public: // Typedefs
  ///
  /// Actions that can be executed for a bible reference the user clicks on.
  /// Which action a click executes is up to the user, the value names are the
  /// values the corresponding setting is stored with.
  /// \note The value names must start with an upper case letter. QML does not expose
  /// enum values that start with a lower case letter, they evaluate to undefined there.
  ///
  enum ClickAction
  {
    LookupBrowser,
    None
  };
  Q_ENUM(ClickAction)

public: // Structors
  explicit BridgeBibleRefOcr(
    std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr> workflowBibleRefOcr,
    const std::shared_ptr<bibstd::workflow::workflow_hotkey>& workflowHotkey,
    const std::shared_ptr<bibstd::workflow::workflow_settings>& workflowSettings,
    bibstd::util::non_owning_ptr<QObject> parent = nullptr
  );
  ~BridgeBibleRefOcr() noexcept override;

public: // Accessors
  ///
  /// Access the action configured for a click on a found bible reference.
  /// \return configured click action
  ///
  Q_INVOKABLE ClickAction clickAction() const;

signals:
  // clang-format off
  void runningChanged(bool running);
  void cursorPositionChanged(const QPoint& cursorPosition);
  void referenceFound(const QString& bookId, int chapter, int verse);
  void referenceRangeFound(const QString& bookId, int chapterBegin, int verseBegin, int chapterEnd, int verseEnd, const QRect& boundingBox);
  // clang-format on

public: // Modifiers
  ///
  /// Disconnect all signal connections.
  /// This will stop the frontend backend communication.
  ///
  void disconnect();

private: // Implementation
  ///
  /// Search a bible reference around the current cursor position and notify the QML layer
  /// about start and result of the search. This is called from the hotkey signal and blocks
  /// the calling thread until the search finished.
  ///
  void findReference();

  ///
  /// Notify the QML layer that a reference search started.
  /// The notification is executed on the thread this object lives on.
  ///
  void notifySearchStarted(bibstd::framework::process_id_type processId);

  ///
  /// Notify the QML layer about the result of a reference search. A search that found no
  /// reference reports no reference range. The bounding box is given in native screen pixels,
  /// it is converted to the coordinate system of the QML layer by the notification itself.
  /// The notification is executed on the thread this object lives on.
  ///
  void notifySearchFinished(
    bibstd::framework::process_id_type processId,
    std::optional<bibstd::bible::reference_range> referenceRange,
    std::optional<bibstd::util::screen_rect_type> boundingBox
  );
};

} // namespace bibqml
