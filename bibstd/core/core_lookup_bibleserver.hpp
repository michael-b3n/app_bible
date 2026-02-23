#pragma once

#include "bibstd/bible/reference_range.hpp"
#include "bibstd/util/const_map.hpp"

#include <string_view>

namespace bibstd::core
{

///
/// Core bibleserver lookup. This class calls the bibleserver URL.
///
class core_lookup_bibleserver final
{
public: // Structors
  core_lookup_bibleserver() = default;
  ~core_lookup_bibleserver() noexcept = default;

public: // Operations
  ///
  /// Open reference range with the bibleserver in the default web browser. Multiple tabs might be opened.
  /// This function does not support reference ranges over multiple books.
  /// \param range Reference range that shall be opened
  /// \return true if successful, false otherwise
  ///
  auto open(const bible::reference_range& range, const std::vector<bible::translation>& translations) -> bool;

private: // Constants
  static constexpr auto translations_map_de = util::make_const_bimap<bible::translation, std::string_view>({
    { bible::translation::dbu,  "DBU"},
    { bible::translation::elb,  "ELB"},
    { bible::translation::esv,  "ESV"},
    {  bible::translation::eu,   "EU"},
    { bible::translation::gnb,  "GNB"},
    { bible::translation::hfa,  "HFA"},
    { bible::translation::kjv,  "KJV"},
    { bible::translation::lut,  "LUT"},
    {bible::translation::meng, "MENG"},
    { bible::translation::neu,  "NeÜ"},
    { bible::translation::ngu,  "NGÜ"},
    {bible::translation::nirv, "NIRV"},
    { bible::translation::niv,  "NIV"},
    { bible::translation::nlb,  "NLB"},
    { bible::translation::slt,  "SLT"},
    { bible::translation::vxb,  "VXB"},
    {  bible::translation::zb,   "ZB"}
  });
  static_assert(translations_map_de.size() == util::enum_count<bible::translation>());
};

} // namespace bibstd::core
