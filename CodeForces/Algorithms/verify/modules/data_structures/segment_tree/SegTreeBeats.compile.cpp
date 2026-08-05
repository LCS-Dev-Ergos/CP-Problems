#include "modules/data_structures/segment_tree/SegTreeBeats.hpp"

int main() {
  // a = {1, 5, 3}; chmin to 4 gives {1, 4, 3}; add 2 gives {3, 6, 5}.
  BeatsSumMinMax<I64> seg(VecI64{1, 5, 3});
  seg.chmin(0, 3, 4);
  seg.add(0, 3, 2);
  const auto [sum, lo, hi] = seg.query(0, 3);
  const bool ok = (sum == 14) && (lo == 3) && (hi == 6) && (seg.max(0, 2) == 6);
  return ok ? 0 : 1;
}
