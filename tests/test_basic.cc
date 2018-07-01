#include "gtest/gtest.h"
#include "rangemap.h"
#include <vector>

namespace rangemap {

class RangeMapTest : public ::testing::Test {
protected:
  struct TestEntry {
    size_t type;
    uint64_t addr_begin;
    uint64_t addr_end;
  };

  void AddRange(int ind, uint64_t addr, uint64_t size) {
    range_map_.AddRange(ind, addr, size);
  }
  void AssertRangeMap(const std::vector<TestEntry> &ranges) {
    ASSERT_EQ(ranges.size(), range_map_.map_.size());
    auto iter = range_map_.map_.begin();
    size_t i = 0;
    for (; iter != range_map_.map_.end() && i < ranges.size(); ++iter, ++i) {
      ASSERT_EQ(iter->first, ranges[i].addr_begin);
      ASSERT_EQ(iter->first + iter->second.size, ranges[i].addr_end);
      ASSERT_EQ(iter->second.type, ranges[i].type);

    }
  }

  RangeMap range_map_;
};

TEST_F(RangeMapTest, AddSimple) {
  AddRange(1, 10, 10);
  AssertRangeMap({{1, 10, 20}});

  AddRange(2, 30, 10);
  AssertRangeMap({
                  {1, 10, 20},
                  {2, 30, 40}
    });
}

} // namespace rangemap
