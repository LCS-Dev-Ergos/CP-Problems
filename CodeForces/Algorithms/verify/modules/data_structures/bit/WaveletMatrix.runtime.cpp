#include "modules/data_structures/bit/WaveletMatrix.hpp"

#include <cassert>

int main() {
  WaveletMatrix<I32> wm(VecI32{5, 1, 4, 2, 3, 9, 6, 8, 7, 5});

  assert(wm.kth(0, 5, 0) == 1);
  assert(wm.kth(0, 5, 2) == 3);
  assert(wm.kth(2, 9, 4) == 7);
  assert(wm.median_lower(0, 10) == 5);
  assert(wm.median_upper(0, 10) == 5);

  assert(wm.prefix_count(0, 10, 5) == 4);
  assert(wm.count_range(0, 10, 3, 7) == 5);
  assert(wm.count_value(0, 10, 5) == 2);
  assert(wm.count_value(2, 9, 5) == 0);

  const auto min_ge = wm.min_ge(1, 8, 5);
  assert(min_ge.has_value() && *min_ge == 6);
  const auto max_lt = wm.max_lt(1, 8, 5);
  assert(max_lt.has_value() && *max_lt == 4);
  assert(!wm.min_ge(0, 3, 10).has_value());
  assert(!wm.max_lt(0, 3, 1).has_value());
  return 0;
}
