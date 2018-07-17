#include "rangemap.h"

namespace rangemap {

void RangeMap::AddRange(range_type type, size_type addr, size_type size) {
  if (size == 0) {
    return;
  }
  if (IsUnknownSize(size)) {
    AddRangeUnknownSize(type, addr);
  } else {
    AddRangeFixedSize(type, addr, size);
  }
}

void RangeMap::AddRangeRel(range_type type, size_type addr, size_type size,
                           size_type rel_addr) {
  CHECK(rel_addr != kNoRelative);
  // TODO: check overflow
  CHECK(rel_addr + addr >= addr);
  AddRange(type, addr + rel_addr, size);
}

template <class T>
void RangeMap::AddEntry(T it, size_type type, size_type addr, size_type size) {
  if (size == 0)
    return;

  if (!IsUnknownSize(size)) {
    CHECK(addr + size >= addr);
  }

  bool merged_next = false;
  if (!IsEnd(it)) {
    CHECK(GetBegin(it) > addr);
    // Merge into next entry
    if ((type == it->second.type) && (GetBegin(it) == addr + size)) {
      it->second.size += size;
      auto extr = map_.extract(it);
      extr.key() = addr;
      map_.insert(std::move(extr));
      merged_next = true;
    }
  }

  // Merge into prev entry
  if (!IsBegin(it)) {
    auto prev = std::prev(it);
    if ((type == prev->second.type) && (GetEnd(prev) == addr)) {
      if (merged_next) {
        // Collapse with the next region
        prev->second.size += GetSize(it);
        map_.erase(it);
      } else {
        prev->second.size += size;
      }
      return;
    }
  }

  if (!merged_next)
    map_.emplace_hint(it, addr, Entry(type, size));
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
        if ((IsUnknownSize(it)) && (GetBegin(it) < base_beg)) {
          it->second.size = addr - it->first;
        }
      } else {
        base_size = GetBegin(next) - GetEnd(it);
      }
      size_type it_end = GetEnd(it);
      if (IsUnknownSize(it_end)) {
         return;
      }
      base_beg = it_end;
      ++it;
    } else {
      base_size = GetBegin(it) - addr;
    }
  }

  AddEntry(it, type, base_beg, base_size);
}

void RangeMap::AddRangeFixedSize(size_type type, size_type addr, size_type size) {
  auto it = GetContainingOrNext(addr);

  size_type base_beg = addr;
  size_type base_end = addr + size;
  CHECK(base_end > base_beg);
  while (true) {
    // TODO: sanity check for overflow?
    if (IsEnd(it)) {
      AddEntry(it, type, base_beg, base_end - base_beg);
      break;
    } else {
      VerifyEntry(it);
      if (IsEntryContains(it, base_beg)) {
        if (IsUnknownSize(GetEnd(it))) {
          it->second.size = base_end - base_beg;
        } else {
          base_beg = GetEnd(it);
        }
      } else {
        size_type next_beg = GetBegin(it);
        if (base_end > next_beg) {
          AddEntry(it, type, base_beg, next_beg - base_beg);
          base_beg = GetEnd(it);
        } else {
          AddEntry(it, type, base_beg, base_end - base_beg);
          return;
        }
      }
    ++it;
    }

    if (base_beg >= base_end) {
      break;
    }
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
  size_type cov_end = addr + size;
  while (cov_end > addr) {
    if (IsEnd(it) || !IsEntryContains(it, addr)) {
      return false;
    }

    if (IsUnknownSize(it)) {
      return true;
    }
    addr = GetEnd(it);
    ++it;
  }
  return true;
}

bool RangeMap::IsContinious() const {
  size_type prev_end = GetBegin(map_.begin());
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    if (IsUnknownSize(it)) {
      return false;
    }
    size_type new_beg = GetBegin(it);
    if (new_beg != prev_end) {
      return false;
    }
    prev_end = GetEnd(it);
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
      return it;
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
      return it;
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
  return ((addr >= GetBegin(it)) && (GetEnd(it) > addr));
}

template <class T>
void RangeMap::VerifyEntry(T it) const {
  // TODO: strict check overflow
  if (!IsUnknownSize(it)) {
    CHECK(GetBegin(it) + GetSize(it) > GetBegin(it));
  }
  // Pos in mappings
  CHECK(IsEnd(std::next(it)) ||
        GetEnd(it) <= GetBegin(std::next(it)));
  CHECK(IsBegin(it) || GetEnd(std::prev(it)) <= GetBegin(it));
}

} // namespace rangemap
