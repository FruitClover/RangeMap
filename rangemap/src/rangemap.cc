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

template <class T>
void RangeMap::AddEntry(T it, size_type type, size_type addr, size_type size) {
  if (size == 0)
    return;

  if (size != kUnknownSize) {
    CHECK(addr + size >= addr);
  }
  // TODO: strict add before it
  // if (!IsEnd(it)) {
  // printf("Add it: beg = %llu, size = %llu\n",  GetEntryBegin(it), GetEntrySize(it));
  // // TODO: overflow
  //   CHECK(addr <= GetEntryBegin(it));
  // }

  auto added = map_.emplace_hint(it, addr, Entry(type, size));
  MaybeMergeEntry(added);
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
        if (GetEntryEnd(it) == kUnknownSize) {
          it->second.size = base_end - base_beg;
        } else {
          base_beg = GetEntryEnd(it);
        }
      } else {
        size_type next_beg = GetEntryBegin(it);
        if (base_end > next_beg) {
          AddEntry(it, type, base_beg, next_beg - base_beg);
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
  size_type cov_end = addr + size;
  while (cov_end > addr) {
    if (IsEnd(it) || !IsEntryContains(it, addr)) {
      return false;
    }

    if (GetEntrySize(it) == kUnknownSize) {
      return true;
    }
    addr = GetEntryEnd(it);
    ++it;
  }
  return true;
}

bool RangeMap::IsContinious() const {
  size_type prev_end = GetEntryBegin(map_.begin());
  for (auto it = map_.begin(); it != map_.end(); ++it) {
    if (GetEntrySize(it) == kUnknownSize) {
      return false;
    }
    size_type new_beg = GetEntryBegin(it);
    if (new_beg != prev_end) {
      return false;
    }
    prev_end = GetEntryEnd(it);
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
void RangeMap::MaybeMergeEntry(T it) {
  if (!isMergeCommon_ || map_.size() < 2) {
    return;
  }
  if (!IsBegin(it)) {
    auto prev = std::prev(it);
    if ((prev->second.type == it->second.type) &&
        (GetEntryEnd(prev) == GetEntryBegin(it))) {

        size_type add_size = GetEntrySize(it);
        if (add_size == kUnknownSize) {
          prev->second.size = kUnknownSize;
        } else {
          // TODO: check overflow strict
          CHECK(prev->second.size + add_size > prev->second.size);
          prev->second.size += add_size;
        }
        // TODO: CHECK for contains?
        map_.erase(it);
    }
  }

  if (!IsEnd(it)) {
    auto next = std::next(it);
    if (!IsEnd(next)) {
      if ((next->second.type == it->second.type) &&
          (GetEntryEnd(it) == GetEntryBegin(next))) {
        size_type add_size = GetEntrySize(next);
        if (add_size == kUnknownSize) {
          it->second.size = kUnknownSize;
        } else {
          // TODO: check overflow strict
          CHECK(it->second.size + add_size > it->second.size);
          it->second.size += add_size;
        }
        // TODO: CHECK for contains?
        map_.erase(next);
      }
    }
  }
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
