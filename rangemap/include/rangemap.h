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

  void AddRange(range_type type, size_type addr, size_type size);
  bool TryGetType(size_type addr, range_type* type, size_type* size) const;
  bool IsCoverRange(size_type addr, size_type size) const;

private:
  friend class RangeMapTest;
  std::map<size_type, Entry> map_;
};

} // namespace rangemap

#endif // RANGEMAP_INCLUDE_H
