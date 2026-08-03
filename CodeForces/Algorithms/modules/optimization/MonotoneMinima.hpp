#ifndef CP_MODULES_OPTIMIZATION_MONOTONE_MINIMA_HPP
#define CP_MODULES_OPTIMIZATION_MONOTONE_MINIMA_HPP

#include "_Common.hpp"

/**
 * @file MonotoneMinima.hpp
 * @brief Row minima of a totally monotone matrix, held implicitly.
 *
 * @details Both routines receive the matrix through a comparison callback
 * @c select(i,j,k) which is always invoked with @c j<k and answers whether
 * column @c k should replace column @c j as the argmin of row @c i. Passing
 * @c A[i][k]<A[i][j] returns the leftmost minimum of every row, while
 * @c A[i][k]<=A[i][j] returns the rightmost one.
 *
 * The matrix must be totally monotone under the chosen tie-break: the argmin
 * column has to be non-decreasing in the row index, and so must the argmin of
 * every submatrix obtained by deleting rows and columns. A Monge matrix always
 * satisfies this, which is why these two are the engine behind Monge dynamic
 * programming and min-plus convolution.
 *
 * Neither routine materialises the matrix, so the caller pays only for the
 * @c select calls: O((H+W) log H) for @ref monotone_minima and O(H+W) for
 * @ref smawk. The asymptotically worse one is usually the faster in practice,
 * since it does no allocation and touches memory linearly; reach for SMAWK
 * only when @c select itself is expensive.
 */

/// @brief Argmin column of every row, via divide and conquer on the rows.
template <typename F>
[[nodiscard]] auto monotone_minima(I32 h, I32 w, F select) -> VecI32 {
  my_assert(h >= 0);
  VecI32 argmin(h);
  if (h == 0) return argmin;
  my_assert(w > 0);
  // Solving the middle row splits the column range for both halves, because
  // the rows above it cannot look right of its argmin, nor those below left.
  auto solve = [&](auto&& self, I32 row_lo, I32 row_hi, I32 col_lo, I32 col_hi) -> void {
    if (row_lo >= row_hi) return;
    const I32 mid = row_lo + (row_hi - row_lo) / 2;
    I32 best = col_lo;
    FOR(y, col_lo + 1, col_hi) {
      if (select(mid, best, y)) best = y;
    }
    argmin[mid] = best;
    self(self, row_lo, mid, col_lo, best + 1);
    self(self, mid + 1, row_hi, best, col_hi);
  };
  solve(solve, 0, h, 0, w);
  return argmin;
}

/// @brief Argmin column of every row in O(H+W) select calls, via SMAWK.
template <typename F>
[[nodiscard]] auto smawk(I32 h, I32 w, F select) -> VecI32 {
  my_assert(h >= 0);
  if (h == 0) return {};
  my_assert(w > 0);
  auto solve = [&](auto&& self, const VecI32& rows, const VecI32& cols) -> VecI32 {
    const I32 n = isz(rows);
    if (n == 0) return {};

    // Reduce: a column beaten by a later one on row i can only be beaten again
    // on every row past i, so it survives in no row at all and is dropped. The
    // stack height doubles as the row index to test against.
    VecI32 kept;
    kept.reserve(std::min(n, isz(cols)));
    for (const I32 c : cols) {
      while (!kept.empty() && select(rows[isz(kept) - 1], kept.back(), c)) kept.pop_back();
      if (isz(kept) < n) kept.push_back(c);
    }

    // Recurse on half the rows, then interpolate the rest: each skipped row is
    // bracketed by the argmins of its neighbours, and those brackets are
    // disjoint, so the whole sweep costs one pass over the columns.
    VecI32 odd_rows;
    odd_rows.reserve(n / 2);
    for (I32 i = 1; i < n; i += 2) odd_rows.push_back(rows[i]);
    const VecI32 odd_argmin = self(self, odd_rows, kept);

    VecI32 argmin(n);
    FOR(i, isz(odd_argmin)) argmin[2 * i + 1] = odd_argmin[i];
    I32 p = 0;
    for (I32 i = 0; i < n; i += 2) {
      const I32 limit = (i + 1 == n ? cols.back() : argmin[i + 1]);
      I32 best = cols[p];
      while (cols[p] < limit) {
        ++p;
        if (select(rows[i], best, cols[p])) best = cols[p];
      }
      argmin[i] = best;
    }
    return argmin;
  };

  VecI32 rows(h);
  VecI32 cols(w);
  std::iota(all(rows), 0);
  std::iota(all(cols), 0);
  return solve(solve, rows, cols);
}

#endif
