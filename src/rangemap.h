#ifndef RANGEMAP_INCLUDE_H
#define RANGEMAP_INCLUDE_H

#include <map>

namespace rangemap {

class RangeMap {
public:
  typedef unsigned long size_type;
  typedef size_t range_type;

  struct Entry {
    range_type type;
    size_type size;
  };

  void AddRange(range_type type, size_type addr, size_type size);

private:
  std::map<size_type, Entry> map_;
}

} // namespace rangemap

#endif RANGEMAP_INCLUDE_H
