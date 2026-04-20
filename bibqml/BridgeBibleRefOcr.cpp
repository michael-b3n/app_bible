#include "bibqml/BridgeBibleRefOcr.hpp"

#include <bibstd/bible/book_name_variants_de.hpp>

#include <bibstd/system/screen.hpp>
#include <bibstd/util/enum.hpp>
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

      const auto [reference_ranges, first_reference, passage_content] = [&result]
      {
        auto passage_content = std::string{"..."};
        auto reference_ranges = std::vector<bibstd::bible::reference_range>{};
        auto first_reference = std::optional<bibstd::bible::reference>{};
        try
        {
          if(result && result->passage)
          {
            passage_content = result->passage->content;
            reference_ranges = result->reference_ranges;
            first_reference = result->first_reference;
          }
        }
        catch(const std::exception& e)
        {
          LOG_ERROR("exception occurred while unpacking passage: {}", e.what());
        }
        return std::tuple{reference_ranges, first_reference, passage_content};
      }();

      QMetaObject::invokeMethod(
        this,
        [this, process_id, passage_content, reference_ranges, first_reference]()
        {
          if(processId_ == process_id)
          {
            running_ = false;
            htmlPassage_ = QString::fromStdString(passage_content);
            Q_EMIT htmlPassageChanged(htmlPassage_);
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
