#pragma once

#include "bibstd/bible/common.hpp"

#include <concepts>
#include <cstdint>

namespace bibstd::bible
{
// Forward declarations
class versification;

///
/// Bible reference class. This class contains book name, chapter number and verse number.
///
class reference final
{
public: // Typedefs
  template<typename T>
  struct __typesafe_number_template final
  {
    std::uint32_t value;
    constexpr auto operator<=>(const __typesafe_number_template<T>&) const = default;
  };

  using chapter_type = __typesafe_number_template<struct chapter_tag>;
  using verse_type = __typesafe_number_template<struct verse_tag>;

public: // Static constructor
  ///
  /// Create bible reference.
  /// \param book Book name
  /// \param chapter Chapter number
  /// \param verse_number Verse number
  /// \return bible reference or std::nullopt if not valid
  ///
  static auto create(book_id book, chapter_type chapter, verse_type verse, const versification& validator)
    -> std::optional<reference>;

  ///
  /// \see reference::create
  ///
  static auto create(book_id book, std::integral auto chapter, std::integral auto verse, const versification& validator)
    -> std::optional<reference>;

  ///
  /// Create a bible reference without validating the chapter and verse numbers.
  /// This should only be used when the chapter and verse numbers are already known to be valid.
  /// \param book Book name
  /// \param chapter Chapter number
  /// \param verse_number Verse number
  /// \return bible reference
  ///
  static constexpr auto create_unguarded(book_id book, chapter_type chapter, verse_type verse) -> reference;

  ///
  /// \see reference::create_unguarded
  ///
  static constexpr auto create_unguarded(book_id book, std::integral auto chapter, std::integral auto verse) -> reference;

private: // Constructor
  constexpr reference(book_id book, chapter_type chapter, verse_type verse);

public: // Operators
  constexpr auto operator<=>(const reference&) const = default;

public: // Accessors
  constexpr auto book() const -> book_id { return book_; }
  constexpr auto chapter() const -> chapter_type { return chapter_; }
  constexpr auto verse() const -> verse_type { return verse_; }

private: // Variables
  book_id book_;
  chapter_type chapter_;
  verse_type verse_;
};

///
///
auto reference::create(
  const book_id book, const std::integral auto chapter, const std::integral auto verse, const versification& validator
) -> std::optional<reference>
{
  return reference::create(
    book, chapter_type{static_cast<std::uint32_t>(chapter)}, verse_type{static_cast<std::uint32_t>(verse)}, validator
  );
}

///
///
constexpr auto reference::create_unguarded(const book_id book, const chapter_type chapter, const verse_type verse) -> reference
{
  return reference{book, chapter, verse};
}

///
///
///
constexpr auto reference::create_unguarded(const book_id book, const std::integral auto chapter, const std::integral auto verse)
  -> reference
{
  return reference::create_unguarded(
    book, chapter_type{static_cast<std::uint32_t>(chapter)}, verse_type{static_cast<std::uint32_t>(verse)}
  );
}

///
///
constexpr reference::reference(const book_id book, const chapter_type chapter, const verse_type verse)
  : book_{book}
  , chapter_{chapter}
  , verse_{verse}
{
}

} // namespace bibstd::bible

///
///
template<typename T>
struct std::formatter<bibstd::bible::reference::__typesafe_number_template<T>> : std::formatter<std::string>
{
  auto format(const bibstd::bible::reference::__typesafe_number_template<T> e, std::format_context& ctx) const
  {
    return formatter<std::string>::format(std::format("{}", e.value), ctx);
  }
};

///
///
template<>
struct std::formatter<bibstd::bible::reference> : std::formatter<std::string>
{
  auto format(const bibstd::bible::reference e, std::format_context& ctx) const
  {
    return formatter<std::string>::format(
      std::format("{} {}, {}", bibstd::util::enum_name(e.book()), e.chapter(), e.verse()), ctx
    );
  }
};
