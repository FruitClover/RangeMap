#ifndef RANGEMAP_INCLUDE_H
#define RANGEMAP_INCLUDE_H

#include <map>
#include <limits>

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

private:
  typedef std::map<size_type, Entry> Map;
  typedef Map::const_iterator Map_iterator;

  // If size is unknown, return kUnknownSize;
  size_type GetRangeEnd(Map_iterator it);
  // Return entry that contains addr. end() otherwise
  Map_iterator FindContaining(size_type addr);

  friend class RangeMapTest;
  Map map_;
};

} // namespace rangemap

#endif // RANGEMAP_INCLUDE_H
