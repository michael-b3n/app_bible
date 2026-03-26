#include "bibqml/BridgeBibleRefOcr.hpp"

#ifdef emit
  #undef emit
#endif

#include <bibstd/presenter/presenter_bible_ref_ocr.hpp>
#include <bibstd/util/log.hpp>
#include <bibstd/util/numeric_cast.hpp>

#include <QCursor>
#include <QMetaObject>

namespace bibqml
{

///
///
BridgeBibleRefOcr::BridgeBibleRefOcr(bibstd::presenter::presenter_bible_ref_ocr& presenter, QObject* parent)
  : QObject(parent)
  , connections_(std::make_unique<bibstd::signal::connection_store>())
{
  connections_->add_connection(presenter.connect<bibstd::presenter::detail::presenter_bible_ref_ocr_signal_id::started>(
    [this](const auto process_id)
    {
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
    }
  ));

  connections_->add_connection(presenter.connect<bibstd::presenter::detail::presenter_bible_ref_ocr_signal_id::found>(
    [this](const auto process_id, const auto& passage)
    {
      const auto [text, begin_index] = [&]
      {
        auto unpack = std::make_pair(std::string{}, 0);
        try
        {
          if(passage)
          {
            unpack = std::make_pair(passage->content, numeric_cast<int>(passage->begin_index));
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
  ));
}

///
///
BridgeBibleRefOcr::~BridgeBibleRefOcr() noexcept = default;

} // namespace bibqml
