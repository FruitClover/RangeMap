#include "range_test.h"

namespace rangemap {

TEST_F(RangeMapTest, AddRange) {
  AddRange(1, 10, 10);
  AssertRangeMap({
      {1, 10, 20}
    });

  AddRange(2, 30, 10);
  AssertRangeMap({
      {1, 10, 20},
      {2, 30, 40}
    });

  AddRange(1, 22, 5);
  AssertRangeMap({
      {1, 10, 20},
      {1, 22, 27},
      {2, 30, 40}
    });

  // Skip if has no space
  AddRange(2, 32, 8);
  AssertRangeMap({
      {1, 10, 20},
      {1, 22, 27},
      {2, 30, 40}
    });

  // overlap
  AddRange(3, 10, 40);
  AssertRangeMap({
      {1, 10, 20},
      {3, 20, 22},
      {1, 22, 27},
      {3, 27, 30},
      {2, 30, 40},
      {3, 40, 50}
    });

  // Keep types
  AddRange(1, 10, 40);
  AssertRangeMap({
      {1, 10, 20},
      {3, 20, 22},
      {1, 22, 27},
      {3, 27, 30},
      {2, 30, 40},
      {3, 40, 50}
    });

  AddRange(4, 0, 100);
  AssertRangeMap({
      {4, 0, 10},
      {1, 10, 20},
      {3, 20, 22},
      {1, 22, 27},
      {3, 27, 30},
      {2, 30, 40},
      {3, 40, 50},
      {4, 50, 100}
    });
}

TEST_F(RangeMapTest, AddRangeMiddleInsert) {
  AddRange(1, 10, 30);
  AssertRangeMap({
      {1, 10, 40}
    });

  AddRange(2, 50, 30);
  AssertRangeMap({
      {1, 10, 40},
      {2, 50, 80}
    });

  AddRange(3, 100, 50);
  AssertRangeMap({
      {1, 10, 40},
      {2, 50, 80},
      {3, 100, 150}
    });

  AddRange(4, 0, 1);
  AssertRangeMap({
      {4, 0, 1},
      {1, 10, 40},
      {2, 50, 80},
      {3, 100, 150}
    });

  AddRange(4, 45, 1);
  AssertRangeMap({
      {4, 0, 1},
      {1, 10, 40},
      {4, 45, 46},
      {2, 50, 80},
      {3, 100, 150}
    });

  AddRange(4, 80, 1);
  AssertRangeMap({
      {4, 0, 1},
      {1, 10, 40},
      {4, 45, 46},
      {2, 50, 80},
      {4, 80, 81},
      {3, 100, 150}
    });

  AddRange(4, 99, 1);
  AssertRangeMap({
      {4, 0, 1},
      {1, 10, 40},
      {4, 45, 46},
      {2, 50, 80},
      {4, 80, 81},
      {4, 99, 100},
      {3, 100, 150}
    });
}

TEST_F(RangeMapTest, AddRangeUnknownSize) {
  AddRange(1, 10, RangeMap::kUnknownSize);
  AssertRangeMap({
      {1, 10, RangeMap::kUnknownSize}
    });

  AddRange(2, 20, RangeMap::kUnknownSize);
  AssertRangeMap({
      {1, 10, 20},
      {2, 20, RangeMap::kUnknownSize}
    });

  // Overlap unknown size
  AddRange(2, 20, RangeMap::kUnknownSize);
  AssertRangeMap({
      {1, 10, 20},
      {2, 20, RangeMap::kUnknownSize}
    });

  // Overlap known size
  AddRange(0, 10, RangeMap::kUnknownSize);
  AssertRangeMap({
      {1, 10, 20},
      {2, 20, RangeMap::kUnknownSize}
    });

  AddRange(2, 0, RangeMap::kUnknownSize);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, RangeMap::kUnknownSize}
    });

  AddRange(3, 40, RangeMap::kUnknownSize);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, 40},
      {3, 40, RangeMap::kUnknownSize}
    });

  AddRange(4, 50, 10);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, 40},
      {3, 40, 50},
      {4, 50, 60}
    });

  AddRange(5, 100, 50);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, 40},
      {3, 40, 50},
      {4, 50, 60},
      {5, 100, 150}
    });

  AddRange(6, 90, RangeMap::kUnknownSize);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, 40},
      {3, 40, 50},
      {4, 50, 60},
      {6, 90, 100},
      {5, 100, 150}
    });

  AddRange(6, 90, RangeMap::kUnknownSize);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, 40},
      {3, 40, 50},
      {4, 50, 60},
      {6, 90, 100},
      {5, 100, 150}
    });

  AddRange(6, 60, RangeMap::kUnknownSize);
  AssertRangeMap({
      {2, 0, 10},
      {1, 10, 20},
      {2, 20, 40},
      {3, 40, 50},
      {4, 50, 60},
      {6, 60, 90},
      {6, 90, 100},
      {5, 100, 150}
    });
}

TEST_F(RangeMapTest, RangeCover) {
  // [5, 55]
  AddRange(1, 5, 50);
  AssertCover(true, 5, 55);
  AssertCover(false, 0, 5);
  AssertCover(false, 55, 100);

  //  [5, 55] + [20, 60]
  // =[5, 60]
  AddRange(2, 20, 40);
  AssertCover(true, 5, 55);
  AssertCover(true, 20, 60);
  AssertCover(true, 5, 60);
  AssertCover(false, 0, 5);
  AssertCover(false, 60, 100);

  //  [5, 55] + [20, 60] + [70, 80]
  // =[5, 60] + [70, 80]
  AddRange(3, 70, 10);
  AssertCover(true, 5, 55);
  AssertCover(true, 20, 60);
  AssertCover(true, 5, 60);
  AssertCover(true, 70, 80);
  AssertCover(false, 0, 5);
  AssertCover(false, 60, 70);
  AssertCover(false, 80, 100);

  //  [5, 55] + [20, 60] + [70, 80]  + [81,82]
  // =[5, 60] + [70, 80] + [81. 82]
  AddRange(3, 81, 1);
  AssertCover(true, 5, 55);
  AssertCover(true, 20, 60);
  AssertCover(true, 5, 60);
  AssertCover(true, 70, 80);
  AssertCover(true, 81, 82);
  AssertCover(false, 0, 5);
  AssertCover(false, 60, 70);
  AssertCover(false, 80, 81);
  AssertCover(false, 82, 100);

  // Overlap all prev ranges
  AddRange(4, 5, 95);
  AssertRangeMap({
      {1, 5, 55},
      {2, 55, 60},
      {4, 60, 70},
      {3, 70, 80},
      {4, 80, 81},
      {3, 81, 82},
      {4, 82, 100},
    });
  AssertCover(true, 5, 100);
  AssertCover(false, 0, 5);
  AssertCover(false, 100, 120);
}

TEST_F(RangeMapTest, RangeCoverUnknownSize) {
  AddRange(1, 10, RangeMap::kUnknownSize);
  AssertRangeMap({
      {1, 10, RangeMap::kUnknownSize}
    });
  AssertCover(true, 10, 50);
  AssertCover(false, 0, 10);

  AddRange(2, 20, RangeMap::kUnknownSize);
  AssertRangeMap({
      {1, 10, 20},
      {2, 20, RangeMap::kUnknownSize}
    });
  AssertCover(true, 10, 20);
  AssertCover(true, 20, 50);
  AssertCover(true, 10, 50);
  AssertCover(false, 0, 10);
}

TEST_F(RangeMapTest, GetType) {
  // TODO + FIX: combine with AddRangeTest?
  AddRange(1, 5, 10);
  AssertGetType({1, 5, 15}, 5, 15);

  // TODO: check borders
  AddRange(2, 20, 10);
  AssertGetType({1, 5, 15}, 5, 15);
  AssertGetType({2, 20, 30}, 20, 30);

  AddRange(3, 0, 50);
  AssertGetType({3, 0, 5}, 0, 5);
  AssertGetType({1, 5, 15}, 5, 15);
  AssertGetType({3, 15, 20}, 15, 20);
  AssertGetType({2, 20, 30}, 20, 30);
  AssertGetType({3, 30, 50}, 30, 50);

  // Do not override type
  AddRange(4, 5, 10);
  AddRange(4, 20, 10);
  AddRange(4, 0, 50);
  AssertGetType({3, 0, 5}, 0, 5);
  AssertGetType({1, 5, 15}, 5, 15);
  AssertGetType({3, 15, 20}, 15, 20);
  AssertGetType({2, 20, 30}, 20, 30);
  AssertGetType({3, 30, 50}, 30, 50);
}

TEST_F(RangeMapTest, Gaps) {
  // TODO: test gaps finding
  return;
  ASSERT_TRUE(false);
}

TEST_F(RangeMapTest, Order) {
  // TODO: check adding order?
}

} // namespace rangemap
