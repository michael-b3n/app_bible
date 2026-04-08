#include "bibqml/BridgeBibleRefOcr.hpp"

#include <bibstd/system/screen.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/numeric_cast.hpp>
#include <bibstd/workflow/workflow_bible_ref_ocr.hpp>
#include <bibstd/workflow/workflow_hotkey.hpp>

#include <QCursor>
#include <QMetaObject>

namespace bibqml
{
// Constants
constexpr auto ocr_find_path = "ocr";

///
///
BridgeBibleRefOcr::BridgeBibleRefOcr(
  std::shared_ptr<bibstd::workflow::workflow_bible_ref_ocr> workflow_bible_ref_ocr,
  std::shared_ptr<bibstd::workflow::workflow_hotkey> workflow_hotkey,
  QObject* parent
)
  : QObject(parent)
  , findReferenceSig_{workflow_hotkey->register_callback(ocr_find_path)}
  , connections_(std::make_unique<bibstd::signal::scoped_connection_guard>())
{
  connections_->connect(
    *findReferenceSig_,
    [this, workflow_bible_ref_ocr]()
    {
      const auto cursor_pos = bibstd::system::screen::cursor_position();
      const auto process_id = bibstd::framework::process_id_type{};

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

      const auto result = workflow_bible_ref_ocr->find({{cursor_pos}});

      const auto [text, begin_index] = [&]
      {
        auto unpack = std::make_pair(std::string{}, 0);
        try
        {
          if(result && result->passage)
          {
            unpack = std::make_pair(result->passage->content, numeric_cast<int>(result->passage->begin_index));
          }
        }
        catch(const std::exception& e)
        {
          LOG_ERROR("exception occurred while unpacking passage: {}", e.what());
        }
        return unpack;
      }();

      QMetaObject::invokeMethod(
        this,
        [this, process_id, text, begin_index]()
        {
          if(running_ && processId_ == process_id)
          {
            running_ = false;
            htmlPassage_ = QString::fromStdString(text);
            htmlPassageBeginIndex_ = begin_index;
            Q_EMIT runningChanged(running_);
            Q_EMIT htmlPassageChanged(htmlPassage_);
            Q_EMIT htmlPassageBeginIndexChanged(htmlPassageBeginIndex_);
          }
        },
        Qt::QueuedConnection
      );
    }
  );

  workflow_hotkey->assign_hotkey(
    {ocr_find_path, bibstd::system::hotkey_common::key_modifier::alt, bibstd::system::hotkey_common::key::vk_f}
  );
}

///
///
BridgeBibleRefOcr::~BridgeBibleRefOcr() noexcept = default;

///
///
auto BridgeBibleRefOcr::disconnect() -> void
{
  connections_->disconnect();
}

} // namespace bibqml
