#include "rangemap.h"
#include "utils.h"

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
  auto it = FindContainingOrNext(addr);

  size_type base_beg = addr;
  size_type base_end = addr + size;
  CHECK(base_end > base_beg);
  while (true) {
    // TODO: sanity check for overflow?
    if (IsEnd(it)) {
      map_.emplace_hint(it, base_beg, Entry(type, base_end - base_beg));
      break;
    } else {
      VerifyEntry(it);
      if (IsEntryContains(it, base_beg)) {
        base_beg = GetEntryEnd(it);
      } else {
        size_type next_beg = it->first;
        if (base_end > next_beg) {
          map_.emplace_hint(it, base_beg, Entry(type, next_beg - base_beg));
          base_beg = GetEntryEnd(it);
        }
      }
    }

    if (base_beg >= base_end) {
      break;
    }
    ++it;
  }
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
  CHECK(end >= size);
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

template<class T>
RangeMap::size_type RangeMap::GetEntryEnd(T it) {
  CHECK(!IsEnd(it));
  if (it->second.size == kUnknownSize) {
    return kUnknownSize;
  }
  size_type end = it->first + it->second.size;
  // TODO: check overflow precisely
  CHECK(end >= it->first);
  return end;
}

RangeMap::Map::const_iterator RangeMap::FindContainingOrNext(size_type addr) {
  // X      X      X    X       X    X        X      X    X       X
  //     A-------       B--------    C---------------D-------
  // A      A      B    B       C    C        C      D    D       -

  //  X
  //
  //  -

  // X        X
  // A-----
  // A        -

  // First element whose key goes after addr (or return end())
  // X      X      X    X       X    X        X      X    X       X
  //     A-------       B--------    C---------------D-------
  // A      B      B    C       C    D        D      end  end     end
  auto it = map_.upper_bound(addr); // O(log N)
  if (it != map_.begin()) {
    // if prev entry contains addr -> return prev entry
    // Get prev:
    // X      X      X    X       X    X        X      X    X       X
    //     A-------       B--------    C---------------D-------
    // A      A      A    B       B    C        C      D    D       D
    // TODO: simplified
    auto prev = std::prev(it);
    if (IsEntryContains(prev, addr)) {
      return prev;
    } else {
      return std::next(prev);
    }
  } else {
    return it;
  }
}

template<class T>
bool RangeMap::IsEntryContains(T it, size_type addr) {
  return ((addr >= it->first) && (GetEntryEnd(it) > addr));
}

template<class T>
void RangeMap::VerifyEntry(T it) {
  // TODO: strict check overflow
  CHECK(it->first + it->second.size > it->first);
  // Pos in mappings
  CHECK(std::next(it) == map_.end() || GetEntryEnd(it) <= std::next(it)->first);
  CHECK(it == map_.begin() ||
        GetEntryEnd(std::prev(it)) <= it->first);
}

} // namespace rangemap
