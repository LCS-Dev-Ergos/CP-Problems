#include "modules/data_structures/offline/PointAddRectangleSum.hpp"

#include <cassert>
#include <random>

namespace {

struct Point {
  I32 x;
  I32 y;
  I64 weight;
};

struct Rect {
  I32 xl;
  I32 xr;
  I32 yl;
  I32 yr;
};

/// Reference: scan every point for every rectangle.
auto brute(const Vec<Point>& points, const Vec<Rect>& rects) -> VecI64 {
  VecI64 answer(isz(rects), 0);
  FOR(q, isz(rects)) {
    const Rect& box = rects[q];
    for (const Point& p : points) {
      if (box.xl <= p.x && p.x < box.xr && box.yl <= p.y && p.y < box.yr) {
        answer[q] += p.weight;
      }
    }
  }
  return answer;
}

}  // namespace

int main() {
  std::mt19937 rng(20260810);

  // Small randomized stress, including empty boxes and repeated coordinates.
  FOR(iter, 400) {
    const I32 n = I32(rng() % 25);
    const I32 q = 1 + I32(rng() % 25);
    const I32 span = 1 + I32(rng() % 8);

    Vec<Point> points(n);
    FOR(i, n) {
      points[i] = {I32(rng() % U32(span)) - span / 2, I32(rng() % U32(span)) - span / 2,
                   I64(rng() % 21) - 10};
    }
    Vec<Rect> rects(q);
    FOR(i, q) {
      const I32 x0 = I32(rng() % U32(span + 1)) - span / 2;
      const I32 x1 = I32(rng() % U32(span + 1)) - span / 2;
      const I32 y0 = I32(rng() % U32(span + 1)) - span / 2;
      const I32 y1 = I32(rng() % U32(span + 1)) - span / 2;
      rects[i] = {std::min(x0, x1), std::max(x0, x1), std::min(y0, y1), std::max(y0, y1)};
    }

    PointAddRectangleSum<I64> solver;
    for (const Point& p : points) solver.add_query(p.x, p.y, p.weight);
    for (const Rect& r : rects) solver.sum_query(r.xl, r.xr, r.yl, r.yr);
    assert(solver.update_count() == n);
    assert(solver.query_count() == q);

    const VecI64 got = solver.calc();
    assert(got == brute(points, rects));
    // calc() must not consume the collected data.
    assert(solver.calc() == got);
  }

  // Growing the instance between calls keeps working.
  {
    PointAddRectangleSum<I64> solver;
    solver.add_query(1, 1, 4);
    solver.sum_query(0, 5, 0, 5);
    assert(solver.calc() == (VecI64{4}));
    solver.add_query(2, 2, 6);
    solver.sum_query(2, 3, 2, 3);
    assert(solver.calc() == (VecI64{10, 6}));
  }

  // Degenerate instances answer with the additive identity.
  {
    PointAddRectangleSum<I64> empty_points;
    empty_points.sum_query(0, 10, 0, 10);
    assert(empty_points.calc() == (VecI64{0}));

    PointAddRectangleSum<I64> empty_queries;
    empty_queries.add_query(1, 2, 7);
    assert(empty_queries.calc().empty());

    PointAddRectangleSum<I64> degenerate;
    degenerate.add_query(3, 3, 9);
    degenerate.sum_query(3, 3, 0, 10);  // Empty in x.
    degenerate.sum_query(0, 10, 3, 3);  // Empty in y.
    degenerate.sum_query(3, 4, 3, 4);   // The single cell holding the point.
    assert(degenerate.calc() == (VecI64{0, 0, 9}));
  }

  // Scale case cross-checked against a dense two-dimensional prefix sum.
  {
    constexpr I32 SIDE = 300;
    constexpr I32 N = 100000;
    constexpr I32 Q = 100000;

    Vec2D<I64> grid(SIDE + 1, VecI64(SIDE + 1, 0));
    PointAddRectangleSum<I64> solver;
    FOR(i, N) {
      const I32 x = I32(rng() % U32(SIDE));
      const I32 y = I32(rng() % U32(SIDE));
      const I64 w = I64(rng() % 2001) - 1000;
      solver.add_query(x, y, w);
      grid[x + 1][y + 1] += w;
    }
    FOR(x, SIDE) FOR(y, SIDE) {
      grid[x + 1][y + 1] += grid[x][y + 1] + grid[x + 1][y] - grid[x][y];
    }

    Vec<Rect> rects(Q);
    FOR(i, Q) {
      const I32 x0 = I32(rng() % U32(SIDE + 1));
      const I32 x1 = I32(rng() % U32(SIDE + 1));
      const I32 y0 = I32(rng() % U32(SIDE + 1));
      const I32 y1 = I32(rng() % U32(SIDE + 1));
      rects[i] = {std::min(x0, x1), std::max(x0, x1), std::min(y0, y1), std::max(y0, y1)};
      solver.sum_query(rects[i].xl, rects[i].xr, rects[i].yl, rects[i].yr);
    }

    const VecI64 got = solver.calc();
    assert(isz(got) == Q);
    FOR(i, Q) {
      const Rect& b = rects[i];
      const I64 want =
          grid[b.xr][b.yr] - grid[b.xl][b.yr] - grid[b.xr][b.yl] + grid[b.xl][b.yl];
      assert(got[i] == want);
    }
  }

  return 0;
}
