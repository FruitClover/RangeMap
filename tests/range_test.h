#ifndef RANGEMAP_TEST_INCLUDE
#define RANGEMAP_TEST_INCLUDE

#include "rangemap.h"
#include "gtest/gtest.h"
#include <vector>

namespace rangemap {

class RangeMapTest : public ::testing::Test {
protected:
  struct TestEntry {
    TestEntry(size_t type_, uint64_t beg_, uint64_t end_)
        : type(type_), beg(beg_), end(end_), rel_addr(RangeMap::kNoRelative) {}
    TestEntry(size_t type_, uint64_t beg_, uint64_t end_, uint64_t rel_addr_)
        : type(type_), beg(beg_), end(end_), rel_addr(rel_addr_) {}
    size_t type;
    uint64_t beg;
    uint64_t end;
    uint64_t rel_addr;
  };

  void AddRange(int ind, uint64_t addr, uint64_t size) {
    range_map_.AddRange(ind, addr, size);
  }

  void AddRangeRel(int ind, uint64_t addr, uint64_t size, uint64_t rel_addr) {
    range_map_.AddRangeRel(ind, addr, size, rel_addr);
  }

  void AssertConsistency() {
    uint64_t prev_end = 0;
    for (auto it = range_map_.map_.begin(); it != range_map_.map_.end(); ++it) {
      ASSERT_GE(it->first, prev_end);
      prev_end = range_map_.GetEnd(it);
    }
  }

  void AssertRangeMap(const std::vector<TestEntry> &ranges) {
    AssertConsistency();
    ASSERT_EQ(ranges.size(), range_map_.map_.size());
    auto iter = range_map_.map_.begin();
    size_t i = 0;
    for (; iter != range_map_.map_.end() && i < ranges.size(); ++iter, ++i) {
      EXPECT_EQ(ranges[i].beg, iter->first);
      EXPECT_EQ(ranges[i].end, range_map_.GetEnd(iter));
      EXPECT_EQ(ranges[i].type, iter->second.type);
      EXPECT_EQ(ranges[i].rel_addr, iter->second.rel_addr);
    }
  }

  void AssertCover(bool is_actually_covered, uint64_t beg, uint64_t end) {
    AssertConsistency();
    ASSERT_GT(end, beg);
    for (uint64_t b = beg; b <= end; ++b) {
      for (uint64_t e = end; e > b; --e) {
        ASSERT_EQ(is_actually_covered, range_map_.IsRangeCovered(b, e - b));
      }
    }
  }

  void AssertGetType(TestEntry entry, uint64_t beg, uint64_t end) {
    AssertConsistency();
    ASSERT_GE(end, beg);
    for (uint64_t addr = beg; addr < end; ++addr) {
      uint64_t t, sz;
      bool is_found = range_map_.TryGetEntry(addr, &t, &sz);
      ASSERT_TRUE(is_found);
      EXPECT_EQ(entry.type, t);
      // TODO: check all other outputs
    }
  }
  void AssertContinious(bool is_continious) {
    AssertConsistency();
    EXPECT_EQ(is_continious, range_map_.IsContinious());
  }

  RangeMap range_map_;
};

} // namespace rangemap

#endif // RANGEMAP_TEST_INCLUDE
