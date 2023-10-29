#pragma once

#include <string_view>

namespace bibstd::bible
{

///
///
///
template<std::size_t N>
class bible_book final
{
public: // Typedefs
  using name_type = std::array<std::u8string_view, N>;

public: // Constructor
  ///
  /// Bible book.
  /// \tparam ...Args u6 string view type
  /// \param ...args bible book
  ///
  template<typename... Args>
  constexpr bible_book(Args&&... args);

private: // Variables
  const name_type book_names;
};

///
///
template<typename... Args>
bible_book(Args&&...) -> bible_book<sizeof...(Args)>;

} // namespace bibstd::bible
