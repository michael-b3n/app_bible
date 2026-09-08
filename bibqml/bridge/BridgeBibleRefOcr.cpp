#include "bibqml/bridge/BridgeBibleRefOcr.hpp"

#include <bibstd/framework/setting.hpp>
#include <bibstd/framework/thread_pool.hpp>
#include <bibstd/math/rect.hpp>
#include <bibstd/system/screen.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/numeric_cast.hpp>
#include <bibstd/util/timer.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr_auto.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>
#include <bibstd/workflow/workflow_settings.hpp>

#include <QCursor>
#include <QGuiApplication>
#include <QMetaObject>
#include <QScreen>

#include <algorithm>
#include <cmath>
#include <ranges>
#include <utility>

namespace bibqml
{
namespace
{

///
/// Screen area captured for a reference search. Besides the captured image this contains the
/// cursor position relative to the image origin and the origin of the image on the screen.
///
struct CaptureResult final
{
  bibstd::util::pixel_plane_type image;
  bibstd::util::screen_coordinates_type relativeCursorPosition;
  bibstd::util::screen_coordinates_type origin;
};

///
/// Mapping of one monitor between the two coordinate systems in play:
/// - native screen pixels, the system the backend captures and searches the screen in
/// - device independent pixels, the system the QML layer positions its windows in
/// Both systems describe the same monitors, but a monitor has a different origin in each of
/// them: Qt lays out its screens on its own if they are scaled differently. The origin of a
/// monitor can therefore not be converted, it has to be known in both systems. Everything
/// else is mapped relative to it.
///
struct MonitorMapping final
{
  // Variables
  bibstd::util::screen_coordinates_type nativeOrigin;
  QPoint deviceIndependentOrigin;
  qreal devicePixelRatio;

  ///
  /// Map a rectangle that is on this monitor to device independent pixels.
  /// \param rect Rectangle in native screen pixels
  /// \return rectangle in device independent pixels
  ///
  [[nodiscard]] auto map(const bibstd::util::screen_rect_type& rect) const -> QRect;
};

///
///
auto MonitorMapping::map(const bibstd::util::screen_rect_type& rect) const -> QRect
{
  const auto scale = [this](const auto nativePixels)
  { return numeric_cast<int>(std::lround(static_cast<qreal>(nativePixels) / devicePixelRatio)); };
  const auto offset = QPoint{scale(rect.origin().x() - nativeOrigin.x()), scale(rect.origin().y() - nativeOrigin.y())};
  const auto size = QSize{scale(bibstd::math::size(rect.horizontal_range())), scale(bibstd::math::size(rect.vertical_range()))};
  return QRect{deviceIndependentOrigin + offset, size};
}

///
/// Find the mapping of the monitor showing the specified position. The system and the QML
/// layer identify a monitor by the same platform device name, which is what the monitor of
/// the position is looked up by.
/// \note This accesses the screens of the QML layer, it must be called on its thread.
/// \return monitor mapping, or std::nullopt if the monitor is unknown to the QML layer
///
[[nodiscard]] auto monitorMappingAt(const bibstd::util::screen_coordinates_type& position) -> std::optional<MonitorMapping>
{
  const auto monitor = bibstd::system::screen::monitor_at(position);
  if(!monitor)
  {
    LOG_WARN("no monitor found at position: position={}", position);
    return std::nullopt;
  }
  const auto deviceName = QString::fromStdString(monitor->device_name);
  const auto screens = QGuiApplication::screens();
  const auto screen = std::ranges::find_if(screens, [&](const auto s) { return s->name() == deviceName; });
  if(screen == std::ranges::cend(screens))
  {
    LOG_WARN("no screen found for monitor: device_name=\"{}\"", monitor->device_name);
    return std::nullopt;
  }
  return MonitorMapping{
    .nativeOrigin = monitor->rect.origin(),
    .deviceIndependentOrigin = (*screen)->geometry().topLeft(),
    .devicePixelRatio = (*screen)->devicePixelRatio()
  };
}

///
/// Capture the screen area of the window at the specified cursor position.
/// \return captured screen area, or std::nullopt if the area could not be captured
///
[[nodiscard]] auto captureScreen(const bibstd::util::screen_coordinates_type& cursorPosition) -> std::optional<CaptureResult>
{
  SCOPED_TIMER_LOG();
  const auto windowRect = bibstd::system::screen::window_at(cursorPosition);
  if(!windowRect)
  {
    return std::nullopt;
  }
  auto image = bibstd::util::pixel_plane_type{};
  if(!bibstd::system::screen::capture(*windowRect, image))
  {
    return std::nullopt;
  }
  return CaptureResult{
    .image = std::move(image), .relativeCursorPosition = cursorPosition - windowRect->origin(), .origin = windowRect->origin()
  };
}

///
/// Shift a rectangle given in image coordinates onto the screen the image was captured from.
/// \return rectangle in native screen pixels
///
[[nodiscard]] auto
toScreenRect(const bibstd::util::screen_rect_type& rect, const bibstd::util::screen_coordinates_type& imageOrigin)
  -> bibstd::util::screen_rect_type
{
  return bibstd::util::screen_rect_type{
    rect.origin() + imageOrigin, bibstd::math::size(rect.horizontal_range()), bibstd::math::size(rect.vertical_range())
  };
}

///
/// Create the click action setting, or access it if it exists already. The setting is declared
/// by the frontend since the available actions are a frontend concept: the backend neither
/// knows nor cares which of them is bound to a click.
/// \return non owning pointer to the click action setting
///
[[nodiscard]] auto createClickActionSetting(bibstd::workflow::workflow_settings& workflowSettings)
  -> bibstd::util::non_owning_ptr<bibstd::framework::setting_type_erased<std::string>>
{
  using ClickAction = BridgeBibleRefOcr::ClickAction;
  const auto available = bibstd::util::enum_names<ClickAction>() |
                         std::views::transform([](const auto name) { return std::string{name}; }) |
                         std::ranges::to<std::vector>();
  const auto setting = workflowSettings.type_erased_setting(
    std::string{"ocr.click_action"},
    std::string{bibstd::util::enum_name(ClickAction::LookupBrowser)},
    std::make_shared<bibstd::framework::setting_validator_list<std::string>>(available)
  );
  return std::get<bibstd::util::non_owning_ptr<bibstd::framework::setting_type_erased<std::string>>>(setting);
}

///
/// Create the auto search setting, or access it if it exists already. The setting is declared by
/// the frontend because it is the frontend that states the intent: the backend searches when it is
/// told to and knows nothing about what the user asked for last time.
/// \return non owning pointer to the auto search setting
///
[[nodiscard]] auto createAutoSearchSetting(bibstd::workflow::workflow_settings& workflowSettings)
  -> bibstd::util::non_owning_ptr<bibstd::framework::setting_type_erased<bool>>
{
  const auto setting = workflowSettings.type_erased_setting(std::string{"ocr.auto_search"}, false);
  return std::get<bibstd::util::non_owning_ptr<bibstd::framework::setting_type_erased<bool>>>(setting);
}

} // namespace

// Constants
constexpr auto ocrFindPath = "ocr";

///
///
BridgeBibleRefOcr::BridgeBibleRefOcr(
  std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr> workflowBibleRefOcr,
  std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr_auto> workflowBibleRefOcrAuto,
  const std::shared_ptr<bibstd::workflow::workflow_hotkey>& workflowHotkey,
  const std::shared_ptr<bibstd::workflow::workflow_settings>& workflowSettings,
  const bibstd::util::non_owning_ptr<QObject> parent
)
  : QObject{parent}
  , workflowBibleRefOcr_{std::move(workflowBibleRefOcr)}
  , workflowBibleRefOcrAuto_{std::move(workflowBibleRefOcrAuto)}
  , manualSearchSig_{workflowHotkey->register_callback(ocrFindPath)}
  , clickActionSetting_{createClickActionSetting(*workflowSettings)}
  , autoSearchSetting_{createAutoSearchSetting(*workflowSettings)}
  , autoSearchExecutor_{bibstd::framework::thread_pool::strand_id()}
{
  executor_.connect(*manualSearchSig_, [this]() { runManualSearch(); });
  workflowHotkey->assign_hotkey({
    {ocrFindPath, bibstd::system::hotkey_common::key_modifier::alt, bibstd::system::hotkey_common::key::vk_f}
  });

  workflowBibleRefOcrAuto_->connect_queued(
    &bibstd::workflow::workflow_bible_ref_ocr_auto_sigs::detected,
    [this](const auto& detected)
    {
      // The ranges are ordered canonically, the first one is the reference that was detected.
      if(detected && !detected->reference_ranges.empty())
      {
        notifyAutoSearchDetection(detected->reference_ranges.front(), detected->reference_bounding_box);
      }
    },
    executor_
  );

  autoSearchSetting_->signal_adapter.connect_queued(
    &bibstd::framework::setting_signals::value_changed, [this]() { applyAutoSearch(); }, autoSearchExecutor_
  );
  // An unchanged setting emits nothing, so a search left on is started here. Starting is the one
  // half that does not wait for a run, so it needs no thread of its own
  if(autoSearchSetting_->value())
  {
    applyAutoSearch();
  }
}

///
///
BridgeBibleRefOcr::ClickAction BridgeBibleRefOcr::clickAction() const
{
  const auto action = bibstd::util::to_enum<ClickAction>(clickActionSetting_->value());
  if(!action)
  {
    // A value that cannot be read, e.g. one persisted by an older version, falls back to the
    // default action. Reporting no action instead would leave a click without any effect.
    LOG_WARN("unknown click action configured: value=\"{}\"", clickActionSetting_->value());
    return ClickAction::LookupBrowser;
  }
  return *action;
}

///
///
void BridgeBibleRefOcr::setAutoSearch(const bool enabled)
{
  // The setting is what the search follows, writing it is what starts and stops it
  autoSearchSetting_->value(enabled);
}

///
///
void BridgeBibleRefOcr::disconnect()
{
  executor_.disconnect();
  autoSearchExecutor_.disconnect();
}

///
///
void BridgeBibleRefOcr::runManualSearch()
{
  // Capture the screen directly on call of this function to ensure the cursor position is up-to-date.
  // This is usually called from the main thread and takes only a few milliseconds.
  // This also ensures that no displayed windows are blocking the screen capture.
  const auto cursorPosition = bibstd::system::screen::cursor_position();
  if(!cursorPosition)
  {
    LOG_WARN("identify cursor position failed: not found");
    return;
  }
  const auto capture = captureScreen(*cursorPosition);
  if(!capture)
  {
    LOG_WARN("capture screen failed: cursor_position={}", *cursorPosition);
    return;
  }

  const auto processId = bibstd::framework::process_id_type{};
  notifyManualSearchStarted(processId);

  const auto result = workflowBibleRefOcr_->find({
    {.image = capture->image, .position = capture->relativeCursorPosition}
  });
  if(!result.has_value() || result->reference_ranges.empty())
  {
    notifyManualSearchFinished(processId, std::nullopt, std::nullopt);
    return;
  }

  auto boundingBox = std::optional<bibstd::util::screen_rect_type>{};
  if(result->reference_bounding_box)
  {
    boundingBox = toScreenRect(*result->reference_bounding_box, capture->origin);
  }
  // The ranges are ordered canonically, the first one is the reference the passage belongs to.
  notifyManualSearchFinished(processId, result->reference_ranges.front(), boundingBox);
}

///
///
void BridgeBibleRefOcr::applyAutoSearch()
{
  const auto enabled = autoSearchSetting_->value();
  const auto result = enabled ? workflowBibleRefOcrAuto_->start({}) : workflowBibleRefOcrAuto_->stop({});
  if(result)
  {
    notifyAutoSearchRunning(enabled);
  }
}

///
///
void BridgeBibleRefOcr::setManualSearch(const std::optional<bibstd::framework::process_id_type> processId)
{
  manualSearchProcessId_ = processId;
  const auto running = manualSearchProcessId_.has_value();
  if(manualSearchRunning_ != running)
  {
    manualSearchRunning_ = running;
    emit manualSearchRunningChanged(manualSearchRunning_);
  }
}

///
///
void BridgeBibleRefOcr::notifyManualSearchStarted(const bibstd::framework::process_id_type processId)
{
  QMetaObject::invokeMethod(
    this,
    [this, processId]()
    {
      setManualSearch(processId);
      cursorPosition_ = QCursor::pos();
      emit cursorPositionChanged(cursorPosition_);
    },
    Qt::QueuedConnection
  );
}

///
///
void BridgeBibleRefOcr::notifyManualSearchFinished(
  const bibstd::framework::process_id_type processId,
  const std::optional<bibstd::bible::reference_range> referenceRange,
  const std::optional<bibstd::util::screen_rect_type> boundingBox
)
{
  QMetaObject::invokeMethod(
    this,
    [this, processId, referenceRange, boundingBox]()
    {
      // A newer search took over in the meantime, only its result is of interest.
      if(manualSearchProcessId_ != processId)
      {
        return;
      }
      setManualSearch(std::nullopt);

      if(referenceRange)
      {
        emitReference(*referenceRange, boundingBox);
      }
    },
    Qt::QueuedConnection
  );
}

///
///
void BridgeBibleRefOcr::notifyAutoSearchDetection(
  const bibstd::bible::reference_range referenceRange, const std::optional<bibstd::util::screen_rect_type> boundingBox
)
{
  QMetaObject::invokeMethod(
    this,
    [this, referenceRange, boundingBox]()
    {
      // A manual search that is still in flight is left behind by this detection, so it is no
      // longer the current one and its result is dropped when it arrives
      setManualSearch(std::nullopt);
      cursorPosition_ = QCursor::pos();
      emit cursorPositionChanged(cursorPosition_);
      emitReference(referenceRange, boundingBox);
    },
    Qt::QueuedConnection
  );
}

///
///
void BridgeBibleRefOcr::notifyAutoSearchRunning(const bool running)
{
  QMetaObject::invokeMethod(
    this,
    [this, running]()
    {
      if(autoSearchRunning_ != running)
      {
        autoSearchRunning_ = running;
        emit autoSearchRunningChanged(autoSearchRunning_);
      }
    },
    Qt::QueuedConnection
  );
}

///
///
void BridgeBibleRefOcr::emitReference(
  const bibstd::bible::reference_range& referenceRange, const std::optional<bibstd::util::screen_rect_type>& boundingBox
)
{
  decltype(auto) begin = referenceRange.begin();
  decltype(auto) end = referenceRange.end();
  const auto bookId = QString::fromStdString(std::string{bibstd::util::enum_name(begin.book())});
  const auto mapping = boundingBox ? monitorMappingAt(boundingBox->origin()) : std::optional<MonitorMapping>{};

  emit referenceFound(bookId, numeric_cast<int>(begin.chapter().value), numeric_cast<int>(begin.verse().value));
  emit referenceRangeFound(
    bookId,
    numeric_cast<int>(begin.chapter().value),
    numeric_cast<int>(begin.verse().value),
    numeric_cast<int>(end.chapter().value),
    numeric_cast<int>(end.verse().value),
    mapping ? mapping->map(*boundingBox) : QRect{}
  );
}

} // namespace bibqml
