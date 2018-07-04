#include "rangemap.h"
#include <cassert>

namespace rangemap {

// +     A-----
// =
// >     A-----

// +           B..>
// =
// >     A-----B..>

// +                              C---------------------
// =
// >     A-----B------------------C---------------------

// +                                                         J-------
// =
// >     A-----B------------------C---------------------     J-------

// +       D1------          D2..>
// =
// >     A-----B------------------C---------------------D2---J-------

// +  E..>
// =
// >  E--A-----B------------------C---------------------D2---J-------

// +  F---------------------------------------------------------------------
// =
// >  E--A-----B------------------C---------------------D2---J-------F------
void RangeMap::AddRange(range_type type, size_type addr, size_type size) {
  map_.emplace(addr, Entry(type, size));
  // First element whose key goes after addr (or return end())
  auto it = map_.upper_bound(addr); // O(log n)
}

bool RangeMap::TryGetEntry(size_type addr, range_type *type,
                           size_type *size) const {
  return false;
}

bool RangeMap::IsRangeCovered(size_type addr, size_type size) const {
  auto it = map_.upper_bound(addr);
  if (it != map_.begin()) {
    --it;
  }

  size_type beg = addr;
  size_type end = addr + size;
  assert(end >= size);
  while(end > beg) {
    beg += it->second.size;
    if (it == map_.end())
      return false;
    // There is no element on the right side
    // x..> covers [addr, addr + size]
    if (it->second.size == kUnknownSize) {
      return true;
    }
  }
  return true;
}

RangeMap::size_type RangeMap::GetRangeEnd(Map_iterator it) {
  if (it->second.size == kUnknownSize) {
    return kUnknownSize;
  }
  size_type end = it->first + it->second.size;
  // TODO: check overflow precisely
  assert(end >= it->first);
  return (it->first + it->second.size);
}

RangeMap::Map_iterator RangeMap::FindContaining(size_type addr) {
  auto it = map_.lower_bound(addr);
}

} // namespace rangemap
