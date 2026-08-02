#include "bibqml/bridge/BridgeBibleRefOcr.hpp"

#include <bibstd/bible/ocr_book_variants_de.hpp>

#include <bibstd/system/screen.hpp>
#include <bibstd/util/enum.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/timer.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>

#include <QCursor>
#include <QMetaObject>

namespace bibqml
{
namespace detail
{

///
/// Result of capturing a screen area.
/// This contains the relative cursor position and the captured pixel plane.
///
struct capture_screen_result_t final
{
  bibstd::util::pixel_plane_type image;
  bibstd::util::screen_coordinates_type relative_cursor_pos;
};

///
/// Capture screen area of the underlying window.
/// \return pixel plane as image and the relative cursor postion in the image
///
[[nodiscard]] auto capture_screen(const bibstd::util::screen_coordinates_type& cursor_position)
  -> std::optional<capture_screen_result_t>
{
  SCOPED_TIMER_LOG();
  const auto window_rect = bibstd::system::screen::window_at(cursor_position);
  if(!window_rect)
  {
    return std::nullopt;
  }
  auto pixel_plane = bibstd::util::pixel_plane_type{};
  auto success = bibstd::system::screen::capture(*window_rect, pixel_plane);
  if(!success)
  {
    return std::nullopt;
  }
  return capture_screen_result_t{std::move(pixel_plane), cursor_position - window_rect->origin()};
}

} // namespace detail

// Constants
constexpr auto ocr_find_path = "ocr";

///
///
BridgeBibleRefOcr::BridgeBibleRefOcr(
  std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr> workflow_bible_ref_ocr,
  std::shared_ptr<bibstd::workflow::workflow_hotkey> workflow_hotkey,
  QObject* parent
)
  : QObject{parent}
  , findReferenceSig_{workflow_hotkey->register_callback(ocr_find_path)}
{
  executor_.connect(
    *findReferenceSig_,
    [this, workflow_bible_ref_ocr]()
    {
      const auto cursor_pos = bibstd::system::screen::cursor_position();
      const auto process_id = bibstd::framework::process_id_type{};

      // Capture screen directly on call of this function to ensure the cursor position is up-to-date.
      // This is usually called from the main thread and takes only a few milliseconds.
      // This also ensures that no displayed windows are blocking the screen capture.
      auto image_data = detail::capture_screen(cursor_pos);
      if(!image_data)
      {
        LOG_WARN("capture screen failed: cursor_position={}", cursor_pos);
        return;
      }

      QMetaObject::invokeMethod(
        this,
        [this, process_id]()
        {
          processId_ = process_id;
          cursorPosition_ = QCursor::pos();
          Q_EMIT cursorPositionChanged(cursorPosition_);
          if(!running_)
          {
            running_ = true;
            Q_EMIT runningChanged(running_);
          }
        },
        Qt::QueuedConnection
      );

      const auto result = workflow_bible_ref_ocr->find({
        {image_data->image, image_data->relative_cursor_pos}
      });
      const auto valid = result.has_value() && result->passage.has_value();
      auto first_reference = valid ? result->first_reference : std::optional<bibstd::bible::reference>{};

      QMetaObject::invokeMethod(
        this,
        [this, process_id, first_reference]()
        {
          if(processId_ == process_id)
          {
            running_ = false;
            Q_EMIT runningChanged(running_);

            if(first_reference.has_value())
            {
              const auto& ref = *first_reference;
              const auto bookName = QString::fromStdString(std::string{bibstd::util::enum_name(ref.book())});
              Q_EMIT referenceFound(bookName, static_cast<int>(ref.chapter().value), static_cast<int>(ref.verse().value));
            }
          }
        },
        Qt::QueuedConnection
      );
    }
  );

  workflow_hotkey->assign_hotkey({
    {ocr_find_path, bibstd::system::hotkey_common::key_modifier::alt, bibstd::system::hotkey_common::key::vk_f}
  });
}

///
///
BridgeBibleRefOcr::~BridgeBibleRefOcr() noexcept = default;

///
///
void BridgeBibleRefOcr::disconnect()
{
  executor_.disconnect();
}

} // namespace bibqml
