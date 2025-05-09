#pragma once

#include "bible/reference_range.hpp"
#include "util/const_bimap.hpp"

#include <string_view>

namespace bibstd::core
{

///
/// Core bibleserver lookup. This class calls the bibleserver URL.
///
class core_bibleserver_lookup final
{
public: // Structors
  core_bibleserver_lookup() = default;
  ~core_bibleserver_lookup() noexcept = default;

public: // Operations
  ///
  /// Open reference range with the bibleserver in the default web browser. Multiple tabs might be opened.
  /// This function does not support reference ranges over multiple books.
  /// \param range Reference range that shall be opened
  /// \return true if successful, false otherwise
  ///
  auto open(const bible::reference_range& range, const std::vector<bible::translation>& translations) -> bool;

private: // Constants
  static constexpr auto translations_map_de = util::const_bimap(
    std::pair{bible::translation::dbu, std::string_view("DBU")},
    std::pair{bible::translation::elb, std::string_view("ELB")},
    std::pair{bible::translation::esv, std::string_view("ESV")},
    std::pair{bible::translation::eu, std::string_view("EU")},
    std::pair{bible::translation::gnb, std::string_view("GNB")},
    std::pair{bible::translation::hfa, std::string_view("HFA")},
    std::pair{bible::translation::kjv, std::string_view("KJV")},
    std::pair{bible::translation::lut, std::string_view("LUT")},
    std::pair{bible::translation::meng, std::string_view("MENG")},
    std::pair{bible::translation::neu, std::string_view("NeÜ")},
    std::pair{bible::translation::ngu, std::string_view("NGÜ")},
    std::pair{bible::translation::nirv, std::string_view("NIRV")},
    std::pair{bible::translation::niv, std::string_view("NIV")},
    std::pair{bible::translation::nlb, std::string_view("NLB")},
    std::pair{bible::translation::slt, std::string_view("SLT")},
    std::pair{bible::translation::vxb, std::string_view("VXB")},
    std::pair{bible::translation::zb, std::string_view("ZB")}
  );
  static_assert(translations_map_de.size() == static_cast<std::size_t>(util::to_integral(bible::translation::END)));
};

} // namespace bibstd::core
