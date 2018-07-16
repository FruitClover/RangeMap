#ifndef RANGEMAP_INCLUDE_H
#define RANGEMAP_INCLUDE_H

#include <limits>
#include <map>
#include "utils.h"

namespace rangemap {

class RangeMap {
 public:
  typedef uint64_t size_type;
  typedef size_t range_type;
  static const size_type kUnknownSize = std::numeric_limits<size_type>::max();

  struct Entry {
    Entry(range_type type_, size_type size_) : type(type_), size(size_) {}
    range_type type;
    size_type size;
  };

  // Insert new entry [addr, addr + size]
  void AddRange(range_type type, size_type addr, size_type size);

  // If addr belongs to some entry, fill type and size for this entry
  bool TryGetEntry(size_type addr, range_type *type, size_type *size) const;

  // Return true is there is no gaps for [addr, addr + size]
  bool IsRangeCovered(size_type addr, size_type size) const;

  // True is there are no gaps in mapping
  bool IsContinious() const;

 private:
  typedef std::map<size_type, Entry> Map;

  template <class T>
  void AddEntry(T it, size_type type, size_type addr, size_type size);

  void AddRangeUnknownSize(size_type type, size_type addr);
  void AddRangeFixedSize(size_type type, size_type addr, size_type size);

  // If size is unknown, return kUnknownSize;
  template <class T>
  size_type GetEnd(T it) const {
    // TODO: accept end to simplified other functions
    CHECK(!IsEnd(it));
    if (it->second.size == kUnknownSize) {
      return kUnknownSize;
    }
    size_type end = it->first + it->second.size;
    // TODO: check overflow precisely
    CHECK(end >= it->first);
    return end;
  }

  template <class T>
  size_type GetEndStrict(T it) const {
    // TODO: accept end to simplified other functions
    if (it->second.size == kUnknownSize) {
      auto next = std::next(it);
      if (IsEnd(next)) {
        return kUnknownSize;
      } else {
        return GetBegin(next);
      }
    } else {
      size_type end = it->first + it->second.size;
      // TODO: check overflow precisely
      CHECK(end >= it->first);
      return end;
    }
  }

  template <class T>
  size_type GetBegin(T it) const {
    // TODO: accept end to simplified other functions
    CHECK(!IsEnd(it));
    return it->first;
  }

  template <class T>
  size_type GetSize(T it) const {
    // TODO: accept end to simplified other functions
    CHECK(!IsEnd(it));
    return it->second.size;
  }

  // Get entry that contains addr or the next one
  Map::const_iterator GetContainingOrNext(size_type addr) const;
  Map::iterator GetContainingOrNext(size_type addr);

  // Get entry that contains addr or end() otherwise
  Map::const_iterator GetContaining(size_type addr) const;

  // True if 'it' has addr
  template <class T>
  bool IsEntryContains(T it, size_type addr) const;

  bool VerifyMappings() const;

  template <class T>
  void UpdateSize(T it, size_type next_addr);

  template <class T>
  bool IsEnd(T it) const {
    return (it == map_.end());
  }

  template <class T>
  bool IsBegin(T it) const {
    return (it == map_.begin());
  }

  template <class T>
  bool IsUnknownSize(T it) const {
    return GetSize(it) == kUnknownSize;
  }

  bool IsUnknownSize(size_type size) const {
    return size == kUnknownSize;
  }

  template <class T>
  void MaybeMergeEntry(T it);

  template <class T>
  void VerifyEntry(T it) const;

  // Find first gap between start and end, return map_.end() if none
  template<class T>
  T FindFirstGap(T start, T end) const;

  friend class RangeMapTest;
  bool isMergeCommon_ = false;
  Map map_;
};

// TODO: impl
// TODO: Merge(vector RangeMap)

}  // namespace rangemap

#endif  // RANGEMAP_INCLUDE_H
