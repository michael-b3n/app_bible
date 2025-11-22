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
          cursorX_ = QCursor::pos().x();
          cursorY_ = QCursor::pos().y();
          Q_EMIT cursorXChanged();
          Q_EMIT cursorYChanged();
          if(!visible_)
          {
            visible_ = true;
            Q_EMIT visibleChanged();
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
          if(visible_ && processId_ == process_id)
          {
            visible_ = false;
            Q_EMIT visibleChanged();
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
