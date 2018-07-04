#ifndef RANGEMAP_UTILS_INCLUDE_H
#define RANGEMAP_UTILS_INCLUDE_H

#include <cassert>

namespace rangemap {

#define CHECK(expr)                                                            \
  do {                                                                         \
    assert(expr);                                                              \
  } while (0)

} // namespace rangemap

#endif //  RANGEMAP_UTILS_INCLUDE_H
