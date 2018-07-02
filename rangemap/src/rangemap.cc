#include "rangemap.h"

namespace rangemap {

void RangeMap::AddRange(range_type type, size_type addr, size_type size) {
  map_.emplace(addr, Entry(type, size));
}

} // namespace rangemap
