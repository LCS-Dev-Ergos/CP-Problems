#ifndef CP_MODULES_OPTIMIZATION_MONGE_DP_HPP
#define CP_MODULES_OPTIMIZATION_MONGE_DP_HPP

#include "LARSCH.hpp"
#include "MonotoneMinima.hpp"

/**
 * @file MongeDP.hpp
 * @brief Shortest paths on a Monge-weighted DAG on the line.
 *
 * @details The setting is a complete DAG on vertices @c 0..N whose edge
 * @c l->r carries weight @c f(l,r), defined for @c 0<=l<r<=N and satisfying
 * the Monge inequality
 *
 *     f(a,c) + f(b,d) <= f(a,d) + f(b,c)   for all a<=b<=c<=d.
 *
 * That single condition collapses the quadratic DP
 * @c dp[r]=min_l(dp[l]+f(l,r)) to linear time, makes the cost of an optimal
 * path convex in the number of edges it uses, and is what lets the fixed
 * edge-count variant be solved by Lagrangian relaxation.
 *
 * Use @ref is_monge while developing to check the hypothesis on small inputs;
 * every routine here silently returns garbage if it does not hold.
 */

/// @brief Brute-force Monge check on the domain [0, n], in O(n^4).
template <typename T, typename F>
[[nodiscard]] auto is_monge(I32 n, F f) -> bool {
  FOR(a, n + 1) FOR(b, a, n + 1) FOR(c, b, n + 1) FOR(d, c, n + 1) {
    const T lhs = f(a, c) + f(b, d);
    const T rhs = f(a, d) + f(b, c);
    if (lhs > rhs) return false;
  }
  return true;
}

/**
 * @brief Shortest path from 0 to every vertex, with no bound on edge count.
 *
 * @details Runs in O(N) calls to @p f. The element type has to be given
 * explicitly, as in @c monge_shortest_path<I64>(n, f).
 *
 * @return @c dp indexed by vertex, with @c dp[0]==0.
 */
template <typename T, typename F>
[[nodiscard]] auto monge_shortest_path(I32 n, F f) -> Vec<T> {
  my_assert(n >= 0);
  Vec<T> dp(n + 1, infinity<T>);
  dp[0] = T(0);
  if (n == 0) return dp;

  // Row i stands for target vertex i+1 and column j for the predecessor, so
  // LARSCH only ever reads columns already settled by an earlier row.
  auto entry = [&](I32 i, I32 j) -> T {
    const I32 r = i + 1;
    if (r <= j) return infinity<T>;
    return dp[j] + f(j, r);
  };
  LARSCH<T, decltype(entry)> larsch(n, entry);
  FOR(r, 1, n + 1) {
    const I32 l = larsch.get_argmin();
    dp[r] = dp[l] + f(l, r);
  }
  return dp;
}

/**
 * @brief Shortest path from 0 to N using exactly @p d edges.
 *
 * @details Applies the Lagrangian relaxation known as the Aliens trick: every
 * edge is surcharged by a multiplier, and the multiplier is binary searched
 * until the unconstrained optimum uses @p d edges. Convexity of the optimum in
 * the edge count, which Monge guarantees, is what makes the dual tight, so the
 * answer is exact even when no multiplier yields exactly @p d edges.
 *
 * Runs in O(N log(f_limit)) calls to @p f, and needs a signed integral @c T.
 *
 * @param f_limit Any bound with @c |f(l,r)|<=f_limit; it only sets the search
 * bracket for the multiplier, so a loose one just costs a few iterations. The
 * multipliers actually probed stay within about @c 1.5*f_limit, so
 * @c f_limit*N has to fit in @c T.
 */
template <typename T, typename F>
[[nodiscard]] auto monge_shortest_path_d_edge(I32 n, I32 d, T f_limit, F f) -> T {
  my_assert(1 <= d && d <= n);
  my_assert(T(0) <= f_limit && f_limit <= infinity<T> / 4);
  if (d == 1) return f(0, n);
  if (d == n) {
    T total = T(0);
    FOR(i, n) total += f(i, i + 1);
    return total;
  }

  Vec<T> dp(n + 1, infinity<T>);
  VecI32 used(n + 1, 0);
  T best = neg_infinity<T>;

  // The surcharge cancels out of every argmin, so it only has to be paid when
  // the distance is written back. Ties resolve to the fewest edges, which is
  // what makes the edge count monotone in the multiplier.
  auto calc = [&](T lambda) -> I32 {
    dp[0] = T(0);
    used[0] = 0;
    auto entry = [&](I32 i, I32 j) -> T {
      const I32 r = i + 1;
      if (r <= j) return infinity<T>;
      return dp[j] + f(j, r);
    };
    LARSCH<T, decltype(entry)> larsch(n, entry);
    FOR(r, 1, n + 1) {
      const I32 l = larsch.get_argmin();
      dp[r] = dp[l] + f(l, r) + lambda;
      used[r] = used[l] + 1;
    }
    chmax(best, dp[n] - lambda * T(d));
    return used[n];
  };

  T lo = -3 * f_limit - 10;
  T hi = 3 * f_limit + 10;
  while (lo + 1 < hi) {
    const T mid = (lo + hi) / 2;
    const I32 edges = calc(mid);
    if (edges == d) break;
    (edges > d ? lo : hi) = mid;
  }
  return best;
}

/**
 * @brief One layer of a layered Monge DP: @c out[r]=min_l(dp[l]+f(l,r)).
 *
 * @details This is the building block for "exactly k transitions" problems
 * where the layer count is small enough to iterate: k applications cost
 * O(kN) calls to @p f. Predecessors are restricted to @c l<r, and states not
 * reached in @p dp stay unreached.
 */
template <typename T, typename F>
[[nodiscard]] auto monge_dp_update(I32 n, const Vec<T>& dp, F f) -> Vec<T> {
  my_assert(isz(dp) == n + 1);
  auto at = [&](I32 r, I32 l) -> T {
    if (l >= r || dp[l] >= infinity<T>) return infinity<T>;
    return dp[l] + f(l, r);
  };
  const VecI32 argmin = smawk(n + 1, n + 1, [&](I32 r, I32 l1, I32 l2) -> bool {
    return at(r, l1) > at(r, l2);
  });
  Vec<T> out(n + 1, infinity<T>);
  FOR(r, n + 1) out[r] = at(r, argmin[r]);
  return out;
}

#endif
