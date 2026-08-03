#ifndef CP_MODULES_OPTIMIZATION_KNUTH_OPTIMIZATION_HPP
#define CP_MODULES_OPTIMIZATION_KNUTH_OPTIMIZATION_HPP

#include "_Common.hpp"

/**
 * @file KnuthOptimization.hpp
 * @brief Interval DP with a monotone split point, in quadratic time.
 *
 * @details Solves the optimal-merge recurrence
 *
 *     dp[i][j] = w(i,j) + min over k in [i, j) of (dp[i][k] + dp[k+1][j]),
 *
 * with @c dp[i][i]==0. When @c w obeys the quadrangle inequality
 * @c w(a,c)+w(b,d)<=w(a,d)+w(b,c) for @c a<=b<=c<=d and is monotone on nested
 * intervals, the best split obeys @c opt[i][j-1]<=opt[i][j]<=opt[i+1][j].
 * Scanning only that window telescopes the cubic loop into a quadratic one.
 *
 * The classic instances are optimal binary search trees and stone merging,
 * where @c w is a range sum and therefore trivially satisfies both conditions.
 * The table costs O(n^2) memory, which is the practical bound on @c n here.
 */
template <typename T, typename F>
[[nodiscard]] auto knuth_interval_dp(I32 n, F w) -> Vec2D<T> {
  my_assert(n >= 0);
  Vec2D<T> dp(n, Vec<T>(n, T(0)));
  if (n == 0) return dp;

  Vec2D<I32> opt(n, VecI32(n, 0));
  FOR(i, n) opt[i][i] = i;
  FOR(width, 2, n + 1) {
    FOR(i, n - width + 1) {
      const I32 j = i + width - 1;
      const I32 k_lo = opt[i][j - 1];
      const I32 k_hi = std::min(opt[i + 1][j], j - 1);
      T best = infinity<T>;
      I32 best_k = k_lo;
      FOR(k, k_lo, k_hi + 1) {
        const T cur = dp[i][k] + dp[k + 1][j];
        if (cur < best) {
          best = cur;
          best_k = k;
        }
      }
      dp[i][j] = best + w(i, j);
      opt[i][j] = best_k;
    }
  }
  return dp;
}

#endif
