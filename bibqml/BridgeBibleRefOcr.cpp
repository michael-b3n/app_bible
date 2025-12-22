#include "bibqml/BridgeBibleRefOcr.hpp"

#ifdef emit
  #undef emit
#endif
#include <bibstd/presenter/presenter_bible_ref_ocr.hpp>

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

  connections_->add_connection(presenter.connect<bibstd::presenter::detail::presenter_bible_ref_ocr_signal_id::ended>(
    [this](const auto process_id, [[maybe_unused]] const auto& references)
    {
      QMetaObject::invokeMethod(
        this,
        [this, process_id]()
        {
          if(running_ && processId_ == process_id)
          {
            running_ = false;
            Q_EMIT runningChanged(running_);
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
