#include "bible/reference.hpp"
#include "util/enum.hpp"
#include "util/exception.hpp"
#include "util/log.hpp"

namespace bibstd::bible
{

///
///
auto reference::create(book_id book, chapter_type chapter, verse_type verse) -> std::optional<reference>
{
  const auto count = verse_count(book, chapter.value);
  if(verse == verse_type{0} || !count || verse > verse_type{*count})
  {
    return std::nullopt;
  }
  return reference{book, chapter, verse};
}

///
///
reference::reference(book_id book, chapter_type chapter, verse_type verse)
  : book_{book}
  , chapter_{chapter}
  , verse_{verse}
  , chapter_count_{chapter_count(book).value()}
  , verse_count_{verse_count(book, chapter.value).value()}
{
}

///
///
auto reference::operator++() & -> reference&
{
  increment();
  return *this;
}

///
///
auto reference::operator++(int) -> reference
{
  auto copy = *this;
  increment();
  return copy;
}

///
///
auto reference::operator--() & -> reference&
{
  decrement();
  return *this;
}

///
///
auto reference::operator--(int) -> reference
{
  auto copy = *this;
  decrement();
  return copy;
}

///
///
auto reference::book() const -> book_id
{
  return book_;
}

///
///
auto reference::chapter() const -> chapter_type
{
  return chapter_;
}

///
///
auto reference::verse() const -> verse_type
{
  return verse_;
}

///
///
auto reference::increment() -> void
{
  if(verse_ < verse_type{verse_count_})
  {
    verse_ = verse_type{verse_.value + 1};
  }
  else if(chapter_ < chapter_type{chapter_count_})
  {
    chapter_ = chapter_type{chapter_.value + 1};
    verse_ = verse_type{1};
    verse_count_ = verse_count(book_, chapter_.value).value();
  }
  else if(util::next(book_) < book_id::END)
  {
    book_ = util::next(book_);
    chapter_ = chapter_type{1};
    verse_ = verse_type{1};
    chapter_count_ = chapter_count(book_).value();
    verse_count_ = verse_count(book_, chapter_.value).value();
  }
}

///
///
auto reference::decrement() -> void
{
  if(verse_ > verse_type{1})
  {
    verse_ = verse_type{verse_.value - 1};
  }
  else if(chapter_ > chapter_type{1})
  {
    verse_count_ = verse_count(book_, chapter_.value).value();
    chapter_ = chapter_type{chapter_.value - 1};
    verse_ = verse_type{verse_count_};
  }
  else if(book_ > util::next(book_id::BEGIN))
  {
    chapter_count_ = chapter_count(book_).value();
    verse_count_ = verse_count(book_, chapter_count_).value();
    book_ = util::prev(book_);
    chapter_ = chapter_type{chapter_count_};
    verse_ = verse_type{verse_count_};
  }
}

} // namespace bibstd::bible
