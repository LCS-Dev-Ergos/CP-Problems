#include "modules/geometry/RectangleUnionArea.hpp"

#include <cassert>
#include <random>

int main() {
  RectangleUnionArea<> empty;
  assert(empty.area() == 0);

  RectangleUnionArea<> nested;
  nested.add_rect(-5, -5, 5, 5);
  nested.add_rect(-2, -2, 2, 2);
  nested.add_rect(-5, -5, 5, 5);
  assert(nested.area() == 100);
  assert(nested.area() == 100);

  std::mt19937 rng(20260812);
  FOR(iter, 500) {
    RectangleUnionArea<> sweep;
    bool covered[12][12]{};
    const I32 n = 1 + I32(rng() % 15);
    FOR(i, n) {
      I32 x1 = I32(rng() % 11) - 5;
      I32 x2 = I32(rng() % 11) - 5;
      I32 y1 = I32(rng() % 11) - 5;
      I32 y2 = I32(rng() % 11) - 5;
      if (x1 == x2 || y1 == y2) continue;
      if (x1 > x2) std::swap(x1, x2);
      if (y1 > y2) std::swap(y1, y2);
      sweep.add_rect(x1, y1, x2, y2);
      FOR(x, x1, x2) FOR(y, y1, y2) covered[x + 5][y + 5] = true;
    }
    I64 expected = 0;
    FOR(x, 11) FOR(y, 11) expected += covered[x][y];
    assert(sweep.area() == expected);
  }
  return 0;
}
