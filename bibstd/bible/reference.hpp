#pragma once

#include "bible/common.hpp"

#include <cstdint>

namespace bibstd::bible
{

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
  static auto create(book_id book, chapter_type chapter, verse_type verse) -> std::optional<reference>;

  ///
  /// \see reference::create
  ///
  template<std::unsigned_integral C, std::unsigned_integral V>
  static auto create(book_id book, C chapter, V verse) -> std::optional<reference>;

private: // Constructor
  reference(book_id book, chapter_type chapter, verse_type verse);

public: // Operators
  auto operator<=>(const reference&) const = default;
  auto operator++() & -> reference&; // pre-increment
  auto operator++(int) -> reference; // post-increment
  auto operator--() & -> reference&; // pre-decrement
  auto operator--(int) -> reference; // post-decrement

public: // Accessors
  auto book() const -> book_id;
  auto chapter() const -> chapter_type;
  auto verse() const -> verse_type;

private: // Implementation
  auto increment() -> void;
  auto decrement() -> void;

private: // Variables
  book_id book_;
  chapter_type chapter_;
  verse_type verse_;
  std::uint32_t chapter_count_;
  std::uint32_t verse_count_;
};

///
///
template<std::unsigned_integral C, std::unsigned_integral V>
auto reference::create(const book_id book, const C chapter, const V verse) -> std::optional<reference>
{
  return reference::create(book, chapter_type{chapter}, verse_type{verse});
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
      std::format("{} {}, {}", bibstd::util::to_string_view(e.book()), e.chapter(), e.verse()), ctx
    );
  }
};
