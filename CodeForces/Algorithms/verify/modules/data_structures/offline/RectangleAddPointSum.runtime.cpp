#include "modules/data_structures/offline/RectangleAddPointSum.hpp"

#include <cassert>
#include <random>

namespace {

struct Rect {
  I32 xl;
  I32 xr;
  I32 yl;
  I32 yr;
  I64 weight;
};

struct Point {
  I32 x;
  I32 y;
};

/// Reference: scan every rectangle for every query point.
auto brute(const Vec<Rect>& rects, const Vec<Point>& points) -> VecI64 {
  VecI64 answer(isz(points), 0);
  FOR(q, isz(points)) {
    const Point& p = points[q];
    for (const Rect& box : rects) {
      if (box.xl <= p.x && p.x < box.xr && box.yl <= p.y && p.y < box.yr) {
        answer[q] += box.weight;
      }
    }
  }
  return answer;
}

}  // namespace

int main() {
  std::mt19937 rng(20260811);

  // Small randomized stress, including empty rectangles and shared coordinates.
  FOR(iter, 400) {
    const I32 n = I32(rng() % 25);
    const I32 q = 1 + I32(rng() % 25);
    const I32 span = 1 + I32(rng() % 8);

    Vec<Rect> rects(n);
    FOR(i, n) {
      const I32 x0 = I32(rng() % U32(span + 1)) - span / 2;
      const I32 x1 = I32(rng() % U32(span + 1)) - span / 2;
      const I32 y0 = I32(rng() % U32(span + 1)) - span / 2;
      const I32 y1 = I32(rng() % U32(span + 1)) - span / 2;
      rects[i] = {std::min(x0, x1), std::max(x0, x1), std::min(y0, y1), std::max(y0, y1),
                  I64(rng() % 21) - 10};
    }
    Vec<Point> points(q);
    FOR(i, q) {
      points[i] = {I32(rng() % U32(span)) - span / 2, I32(rng() % U32(span)) - span / 2};
    }

    RectangleAddPointSum<I64> solver;
    for (const Rect& r : rects) solver.add_query(r.xl, r.xr, r.yl, r.yr, r.weight);
    for (const Point& p : points) solver.sum_query(p.x, p.y);
    assert(solver.update_count() == n);
    assert(solver.query_count() == q);

    const VecI64 got = solver.calc();
    assert(got == brute(rects, points));
    // calc() must not consume the collected data.
    assert(solver.calc() == got);
  }

  // Growing the instance between calls keeps working.
  {
    RectangleAddPointSum<I64> solver;
    solver.add_query(0, 4, 0, 8, 5);
    solver.sum_query(3, 7);
    assert(solver.calc() == (VecI64{5}));
    solver.add_query(3, 9, 6, 9, 2);
    solver.sum_query(4, 7);
    assert(solver.calc() == (VecI64{7, 2}));
  }

  // Degenerate instances answer with the additive identity.
  {
    RectangleAddPointSum<I64> empty_updates;
    empty_updates.sum_query(1, 2);
    assert(empty_updates.calc() == (VecI64{0}));

    RectangleAddPointSum<I64> empty_queries;
    empty_queries.add_query(0, 5, 0, 5, 3);
    assert(empty_queries.calc().empty());

    RectangleAddPointSum<I64> degenerate;
    degenerate.add_query(2, 2, 0, 9, 100);  // Empty in x.
    degenerate.add_query(0, 9, 2, 2, 100);  // Empty in y.
    degenerate.add_query(1, 3, 1, 3, 7);
    degenerate.sum_query(2, 2);
    degenerate.sum_query(3, 3);  // Just outside the half-open corner.
    assert(degenerate.calc() == (VecI64{7, 0}));
  }

  // Every query abscissa to the left of a corner: the corner must be dropped
  // rather than written past the end of the compressed axis.
  {
    RectangleAddPointSum<I64> clipped;
    clipped.add_query(50, 60, 0, 10, 9);
    clipped.sum_query(0, 5);
    clipped.sum_query(1, 5);
    assert(clipped.calc() == (VecI64{0, 0}));
  }

  // Scale case cross-checked against a dense difference grid.
  {
    constexpr I32 SIDE = 300;
    constexpr I32 N = 100000;
    constexpr I32 Q = 100000;

    Vec2D<I64> grid(SIDE + 1, VecI64(SIDE + 1, 0));
    RectangleAddPointSum<I64> solver;
    FOR(i, N) {
      const I32 x0 = I32(rng() % U32(SIDE + 1));
      const I32 x1 = I32(rng() % U32(SIDE + 1));
      const I32 y0 = I32(rng() % U32(SIDE + 1));
      const I32 y1 = I32(rng() % U32(SIDE + 1));
      const I32 xl = std::min(x0, x1);
      const I32 xr = std::max(x0, x1);
      const I32 yl = std::min(y0, y1);
      const I32 yr = std::max(y0, y1);
      const I64 w = I64(rng() % 2001) - 1000;
      solver.add_query(xl, xr, yl, yr, w);
      grid[xl][yl] += w;
      grid[xr][yl] -= w;
      grid[xl][yr] -= w;
      grid[xr][yr] += w;
    }
    FOR(x, SIDE + 1) FOR(y, SIDE + 1) {
      if (x > 0) grid[x][y] += grid[x - 1][y];
      if (y > 0) grid[x][y] += grid[x][y - 1];
      if (x > 0 && y > 0) grid[x][y] -= grid[x - 1][y - 1];
    }

    Vec<Point> points(Q);
    FOR(i, Q) {
      points[i] = {I32(rng() % U32(SIDE)), I32(rng() % U32(SIDE))};
      solver.sum_query(points[i].x, points[i].y);
    }

    const VecI64 got = solver.calc();
    assert(isz(got) == Q);
    FOR(i, Q) assert(got[i] == grid[points[i].x][points[i].y]);
  }

  return 0;
}
