#include "rangemap.h"

namespace rangemap {

void RangeMap::AddRange(range_type type, size_type addr, size_type size) {
  // TODO: Add option to merge regions for the same type
  if (size == kUnknownSize) {
    AddRangeUnknownSize(type, addr);
  } else {
    AddRangeFixedSize(type, addr, size);
  }
}

void RangeMap::AddRangeUnknownSize(size_type type, size_type addr) {
  // Can spawn only 1 range, maybe fix prev entry size
  auto it = GetContainingOrNext(addr);
  size_type base_beg = addr;
  size_type base_size = kUnknownSize;

  if (!IsEnd(it)) {
    if (IsEntryContains(it, addr)) {
      auto next = std::next(it);
      if (IsEnd(next)) {
        // If prev is unknown size --> fix
        if ((GetEntrySize(it) == kUnknownSize) &&
            (GetEntryBegin(it) < base_beg)) {
          it->second.size = addr - it->first;
        }
      } else {
        base_size = GetEntryBegin(next) - GetEntryEnd(it);
      }
      size_type it_end = GetEntryEnd(it);
      if (it_end == kUnknownSize) {
        return;
      }
      base_beg = it_end;
    } else {
      base_size = GetEntryBegin(it) - addr;
    }
  }

  map_.emplace_hint(it, base_beg, Entry(type, base_size));
}

void RangeMap::AddRangeFixedSize(size_type type, size_type addr, size_type size) {
  auto it = GetContainingOrNext(addr);

  // printf("Adding [%u, %u) entry\n", addr, addr + size);
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
        if (GetEntryEnd(it) == kUnknownSize) {
          // If prev is unknown size --> fix
          it->second.size = addr - it->first;
        } else {
          base_beg = GetEntryEnd(it);
        }
      } else {
        size_type next_beg = GetEntryBegin(it);
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
  auto it = GetContaining(addr);
  if (IsEnd(it)) {
    return false;
  } else {
    // TODO: overdose
    CHECK(IsEntryContains(it, addr));
    *type = it->second.type;
    *size = it->second.size;
    return true;
  }
}

bool RangeMap::IsRangeCovered(size_type addr, size_type size) const {
  if (size == 0) {
    return true;
  }
  // TODO: strict check overflow
  CHECK(addr + size > addr);
  auto it = GetContainingOrNext(addr);
  size_type cov_begin = addr;
  size_type covered = 0;
  // TODO: refactor
  while (size > covered) {
    if (IsEnd(it) || !IsEntryContains(it, addr + covered)) {
      return false;
    }

    if (GetEntrySize(it) == kUnknownSize) {
      return true;
    }

    covered += (cov_begin - GetEntryEnd(it));
    cov_begin = GetEntryEnd(it);
    ++it;
  }
  return true;
}

RangeMap::Map::const_iterator RangeMap::GetContainingOrNext(
    size_type addr) const {
  // X      X      X    X       X    X        X      X    X       X
  //     A-------       B--------    C---------------D-------
  // A      A      B    B       C    C        C      D    D       -

  // First element whose key goes after addr (or return end())
  // X      X      X    X       X    X        X      X    X       X
  //     A-------       B--------    C---------------D-------
  // A      B      B    C       C    D        D      end  end     end
  auto it = map_.upper_bound(addr);  // O(log N)
  if (!IsBegin(it)) {
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

RangeMap::Map::iterator RangeMap::GetContainingOrNext(size_type addr) {
  auto it = map_.upper_bound(addr);  // O(log N)
  if (!IsBegin(it)) {
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

RangeMap::Map::const_iterator RangeMap::GetContaining(size_type addr) const {
  auto it = map_.upper_bound(addr);  // O(log N)
  // TODO: simplified
  if (IsBegin(it)) {
    return map_.end();
  }
  --it;
  if (!IsEntryContains(it, addr)) {
    return map_.end();
  }
  return it;
}

template <class T>
bool RangeMap::IsEntryContains(T it, size_type addr) const {
  return ((addr >= GetEntryBegin(it)) && (GetEntryEnd(it) > addr));
}

template <class T>
void RangeMap::VerifyEntry(T it) const {
  // TODO: strict check overflow
  if (GetEntrySize(it) != kUnknownSize) {
    CHECK(GetEntryBegin(it) + GetEntrySize(it) > GetEntryBegin(it));
  }
  // Pos in mappings
  CHECK(IsEnd(std::next(it)) ||
        GetEntryEnd(it) <= GetEntryBegin(std::next(it)));
  CHECK(IsBegin(it) || GetEntryEnd(std::prev(it)) <= GetEntryBegin(it));
}

} // namespace rangemap
