#include "modules/data_structures/offline/RectangleAddRectangleSum.hpp"

#include "modules/data_structures/offline/PointAddRectangleSum.hpp"
#include "modules/data_structures/offline/RectangleAddPointSum.hpp"

#include <cassert>
#include <random>

namespace {

struct Box {
  I64 xl;
  I64 xr;
  I64 yl;
  I64 yr;
};

/// Reference: accumulate the overlap area of each update with each query.
auto brute(const Vec<Box>& rects, const VecI64& weights, const Vec<Box>& queries) -> VecI64 {
  VecI64 answer(isz(queries), 0);
  FOR(q, isz(queries)) {
    const Box& want = queries[q];
    FOR(i, isz(rects)) {
      const Box& box = rects[i];
      const I64 width = std::min(box.xr, want.xr) - std::max(box.xl, want.xl);
      const I64 height = std::min(box.yr, want.yr) - std::max(box.yl, want.yl);
      if (width > 0 && height > 0) answer[q] += weights[i] * width * height;
    }
  }
  return answer;
}

auto random_box(std::mt19937& rng, I64 span) -> Box {
  const I64 x0 = I64(rng() % U32(span + 1));
  const I64 x1 = I64(rng() % U32(span + 1));
  const I64 y0 = I64(rng() % U32(span + 1));
  const I64 y1 = I64(rng() % U32(span + 1));
  return {std::min(x0, x1), std::max(x0, x1), std::min(y0, y1), std::max(y0, y1)};
}

}  // namespace

int main() {
  std::mt19937 rng(20260812);

  // Small randomized stress against the overlap-area reference.
  FOR(iter, 400) {
    const I32 n = I32(rng() % 20);
    const I32 q = 1 + I32(rng() % 20);
    const I64 span = 1 + I64(rng() % 9);

    Vec<Box> rects(n);
    VecI64 weights(n);
    FOR(i, n) {
      rects[i] = random_box(rng, span);
      weights[i] = I64(rng() % 21) - 10;
    }
    Vec<Box> queries(q);
    FOR(i, q) queries[i] = random_box(rng, span);

    RectangleAddRectangleSum<I64> solver;
    FOR(i, n) solver.add_query(rects[i].xl, rects[i].xr, rects[i].yl, rects[i].yr, weights[i]);
    for (const Box& b : queries) solver.sum_query(b.xl, b.xr, b.yl, b.yr);
    assert(solver.update_count() == n);
    assert(solver.query_count() == q);

    const VecI64 got = solver.calc();
    assert(got == brute(rects, weights, queries));
    // calc() must not consume the collected data.
    assert(solver.calc() == got);
  }

  // Negative coordinates, which the closed-form expansion has to handle since
  // it multiplies raw coordinates rather than compressed ranks.
  FOR(iter, 200) {
    const I32 n = 1 + I32(rng() % 12);
    const I32 q = 1 + I32(rng() % 12);

    Vec<Box> rects(n);
    VecI64 weights(n);
    FOR(i, n) {
      Box b = random_box(rng, 12);
      b.xl -= 6, b.xr -= 6, b.yl -= 6, b.yr -= 6;
      rects[i] = b;
      weights[i] = I64(rng() % 21) - 10;
    }
    Vec<Box> queries(q);
    FOR(i, q) {
      Box b = random_box(rng, 12);
      b.xl -= 6, b.xr -= 6, b.yl -= 6, b.yr -= 6;
      queries[i] = b;
    }

    RectangleAddRectangleSum<I64> solver;
    FOR(i, n) solver.add_query(rects[i].xl, rects[i].xr, rects[i].yl, rects[i].yr, weights[i]);
    for (const Box& b : queries) solver.sum_query(b.xl, b.xr, b.yl, b.yr);
    assert(solver.calc() == brute(rects, weights, queries));
  }

  // A single-cell query has to agree with RectangleAddPointSum, and a query of
  // single-cell updates has to agree with PointAddRectangleSum.
  FOR(iter, 200) {
    const I32 n = 1 + I32(rng() % 15);
    const I32 q = 1 + I32(rng() % 15);
    const I64 span = 10;

    RectangleAddRectangleSum<I64> cells;
    RectangleAddPointSum<I64, I64> points;
    RectangleAddRectangleSum<I64> singles;
    PointAddRectangleSum<I64, I64> sparse;

    FOR(i, n) {
      const Box b = random_box(rng, span);
      const I64 w = I64(rng() % 21) - 10;
      cells.add_query(b.xl, b.xr, b.yl, b.yr, w);
      points.add_query(b.xl, b.xr, b.yl, b.yr, w);

      const I64 x = I64(rng() % U32(span));
      const I64 y = I64(rng() % U32(span));
      singles.add_query(x, x + 1, y, y + 1, w);
      sparse.add_query(x, y, w);
    }
    FOR(i, q) {
      const I64 x = I64(rng() % U32(span));
      const I64 y = I64(rng() % U32(span));
      cells.sum_query(x, x + 1, y, y + 1);
      points.sum_query(x, y);

      const Box b = random_box(rng, span);
      singles.sum_query(b.xl, b.xr, b.yl, b.yr);
      sparse.sum_query(b.xl, b.xr, b.yl, b.yr);
    }
    assert(cells.calc() == points.calc());
    assert(singles.calc() == sparse.calc());
  }

  // Degenerate instances answer with the additive identity.
  {
    RectangleAddRectangleSum<I64> empty_updates;
    empty_updates.sum_query(0, 5, 0, 5);
    assert(empty_updates.calc() == (VecI64{0}));

    RectangleAddRectangleSum<I64> empty_queries;
    empty_queries.add_query(0, 5, 0, 5, 3);
    assert(empty_queries.calc().empty());

    RectangleAddRectangleSum<I64> degenerate;
    degenerate.add_query(2, 2, 0, 9, 100);  // Empty in x.
    degenerate.add_query(0, 9, 2, 2, 100);  // Empty in y.
    degenerate.add_query(0, 4, 0, 8, 5);
    degenerate.sum_query(3, 3, 0, 9);  // Empty in x.
    degenerate.sum_query(0, 9, 3, 3);  // Empty in y.
    degenerate.sum_query(2, 6, 3, 9);  // Overlap [2, 4) x [3, 8), 10 cells.
    degenerate.sum_query(0, 4, 0, 8);  // The whole update, 32 cells.
    assert(degenerate.calc() == (VecI64{0, 0, 50, 160}));
  }

  // Scale case cross-checked against a dense grid of cell values.
  {
    constexpr I32 SIDE = 200;
    constexpr I32 N = 60000;
    constexpr I32 Q = 60000;

    Vec2D<I64> cell(SIDE + 1, VecI64(SIDE + 1, 0));
    RectangleAddRectangleSum<I64> solver;
    FOR(i, N) {
      const Box b = random_box(rng, SIDE);
      const I64 w = I64(rng() % 41) - 20;
      solver.add_query(b.xl, b.xr, b.yl, b.yr, w);
      cell[b.xl][b.yl] += w;
      cell[b.xr][b.yl] -= w;
      cell[b.xl][b.yr] -= w;
      cell[b.xr][b.yr] += w;
    }
    // Difference grid to cell values, then cell values to prefix sums.
    FOR(x, SIDE + 1) FOR(y, SIDE + 1) {
      if (x > 0) cell[x][y] += cell[x - 1][y];
      if (y > 0) cell[x][y] += cell[x][y - 1];
      if (x > 0 && y > 0) cell[x][y] -= cell[x - 1][y - 1];
    }
    Vec2D<I64> prefix(SIDE + 1, VecI64(SIDE + 1, 0));
    FOR(x, SIDE) FOR(y, SIDE) {
      prefix[x + 1][y + 1] = cell[x][y] + prefix[x][y + 1] + prefix[x + 1][y] - prefix[x][y];
    }

    Vec<Box> queries(Q);
    FOR(i, Q) {
      queries[i] = random_box(rng, SIDE);
      solver.sum_query(queries[i].xl, queries[i].xr, queries[i].yl, queries[i].yr);
    }

    const VecI64 got = solver.calc();
    assert(isz(got) == Q);
    FOR(i, Q) {
      const Box& b = queries[i];
      const I64 want = prefix[b.xr][b.yr] - prefix[b.xl][b.yr] - prefix[b.xr][b.yl] +
                       prefix[b.xl][b.yl];
      assert(got[i] == want);
    }
  }

  return 0;
}
