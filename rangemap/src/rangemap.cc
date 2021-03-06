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
bool RangeMap::MaybeMergeEntry(T it, size_type type, size_type addr,
                               size_type size) {
  bool is_merged = false;

  if (!IsEnd(it)) {
    // Merge into next entry
    if ((type == GetType(it)) && (GetBegin(it) == addr + size)) {
      AddSize(it, size);
      SetEntryAddress(it, addr);
      is_merged = true;
    }
  }

  // Merge into prev entry
  if (!IsBegin(it)) {
    auto prev = std::prev(it);
    if ((type == GetType(prev)) && (GetEnd(prev) == addr)) {
      // Maybe collapse with the next region
      AddSize(prev, is_merged ? GetSize(it) : size);
      if (is_merged) {
        map_.erase(it);
      }
      is_merged = true;
    }
  }

  return is_merged;
}

template <class T>
void RangeMap::AddEntry(T it, size_type type, size_type addr, size_type size) {
  if (size == 0) return;

  if (!IsUnknownSize(size)) {
    CHECK(addr + size >= addr);
  }

  if (!IsEnd(it)) {
    CHECK(GetBegin(it) > addr);
  }

  if (!MaybeMergeEntry(it, type, addr, size))
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
        // If prev is unknown size --> fix, can be only at the end of the
        // mapping, previous entries are already fixed
        MaybeUpdateUnknownSize(it, base_beg);
      } else {
        base_size = GetBegin(next) - GetEnd(it);
      }
      size_type it_end = GetEnd(it);
      // Mapping unknown size on top of unknown size
      // TODO: Add warning
      if (IsUnknownSize(it_end)) {
        return;
      }
      base_beg = it_end;
      ++it;
    } else {
      // 'it' is the next enrty, calc new fixed size
      base_size = GetBegin(it) - addr;
    }
  }

  AddEntry(it, type, base_beg, base_size);
}

void RangeMap::AddRangeFixedSize(size_type type, size_type addr,
                                 size_type size) {
  CHECK(!IsUnknownSize(size));
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
  CHECK(!IsUnknownSize(addr));
  auto it = GetContaining(addr);
  if (IsEnd(it)) {
    return false;
  } else {
    // TODO: overdose
    CHECK(IsEntryContains(it, addr));
    *type = GetType(it);
    *size = GetSize(it);
    return true;
  }
}

bool RangeMap::IsRangeCovered(size_type addr, size_type size) const {
  CHECK(!IsUnknownSize(size));
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
    if (IsUnknownSize(it) || (GetBegin(it) != prev_end)) {
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
void RangeMap::MaybeUpdateUnknownSize(T it, size_type next_addr) {
  CHECK(!IsUnknownSize(next_addr));
  if ((IsUnknownSize(it)) && (GetBegin(it) < next_addr)) {
    it->second.size = next_addr - it->first;
  }
}

template <class T>
void RangeMap::VerifyEntry(T it) const {
  // TODO: strict check overflow
  if (!IsUnknownSize(it)) {
    CHECK(GetBegin(it) + GetSize(it) > GetBegin(it));
  }
  // Pos in mappings
  CHECK(IsEnd(std::next(it)) || GetEnd(it) <= GetBegin(std::next(it)));
  CHECK(IsBegin(it) || GetEnd(std::prev(it)) <= GetBegin(it));
}

}  // namespace rangemap
