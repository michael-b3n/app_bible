#include "bibstd/bible/passage_info.hpp"

namespace bibstd::bible
{

///
///
passage_info::passage_info(const bible::reference reference, const bible::translation translation)
  : reference(reference)
  , translation(translation)
{
}

} // namespace bibstd::bible
