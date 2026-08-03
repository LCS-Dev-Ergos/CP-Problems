#include "modules/optimization/MongeDP.hpp"

#include <cassert>
#include <random>

namespace {

/// Builds a Monge cost table on the vertices 0..n, as a matrix over (l, r).
auto random_monge(I32 n, std::mt19937& rng) -> Vec2D<I64> {
  Vec2D<I64> g(n + 1, VecI64(n + 1, 0));
  FOR(j, n + 1) g[0][j] = I64(rng() % 50);
  FOR(i, 1, n + 1) g[i][0] = I64(rng() % 50);
  FOR(i, n) {
    FOR(j, n) {
      g[i + 1][j + 1] = g[i][j + 1] + g[i + 1][j] - g[i][j] - I64(rng() % 3);
    }
  }
  return g;
}

auto abs_bound(const Vec2D<I64>& g) -> I64 {
  I64 best = 0;
  for (const VecI64& row : g) {
    for (const I64 v : row) chmax(best, v < 0 ? -v : v);
  }
  return best;
}

/// Quadratic reference for monge_shortest_path.
auto brute_shortest_path(I32 n, const Vec2D<I64>& g) -> VecI64 {
  VecI64 dp(n + 1, infinity<I64>);
  dp[0] = 0;
  FOR(r, 1, n + 1) {
    FOR(l, r) chmin(dp[r], dp[l] + g[l][r]);
  }
  return dp;
}

/// Cubic reference for the fixed edge-count variant.
auto brute_d_edge(I32 n, I32 d, const Vec2D<I64>& g) -> I64 {
  Vec2D<I64> dp(d + 1, VecI64(n + 1, infinity<I64>));
  dp[0][0] = 0;
  FOR(k, 1, d + 1) {
    FOR(r, 1, n + 1) {
      FOR(l, r) {
        if (dp[k - 1][l] < infinity<I64>) chmin(dp[k][r], dp[k - 1][l] + g[l][r]);
      }
    }
  }
  return dp[d][n];
}

}  // namespace

int main() {
  std::mt19937 rng(20260809);

  FOR(iter, 200) {
    const I32 n = 1 + I32(rng() % 20);
    const Vec2D<I64> g = random_monge(n, rng);
    auto f = [&](I32 l, I32 r) -> I64 { return g[l][r]; };

    assert(is_monge<I64>(n, f));
    assert(monge_shortest_path<I64>(n, f) == brute_shortest_path(n, g));

    const I32 d = 1 + I32(rng() % n);
    const I64 limit = abs_bound(g);
    assert(monge_shortest_path_d_edge<I64>(n, d, limit, f) == brute_d_edge(n, d, g));
  }

  // monge_dp_update against the quadratic layer update, including states that
  // the previous layer could not reach.
  FOR(iter, 200) {
    const I32 n = 1 + I32(rng() % 18);
    const Vec2D<I64> g = random_monge(n, rng);
    auto f = [&](I32 l, I32 r) -> I64 { return g[l][r]; };

    VecI64 dp(n + 1);
    FOR(i, n + 1) {
      dp[i] = (rng() % 5 == 0 ? infinity<I64> : I64(rng() % 200));
    }
    dp[0] = 0;

    VecI64 want(n + 1, infinity<I64>);
    FOR(r, n + 1) {
      FOR(l, r) {
        if (dp[l] < infinity<I64>) chmin(want[r], dp[l] + g[l][r]);
      }
    }
    assert(monge_dp_update<I64>(n, dp, f) == want);
  }

  // is_monge must reject a table that violates the inequality.
  {
    auto bad = [](I32 l, I32 r) -> I64 { return -I64(r - l) * I64(r - l); };
    assert(!is_monge<I64>(4, bad));
  }

  // Larger instance, where the quadratic reference is still affordable.
  {
    const I32 n = 2000;
    const Vec2D<I64> g = random_monge(n, rng);
    auto f = [&](I32 l, I32 r) -> I64 { return g[l][r]; };
    assert(monge_shortest_path<I64>(n, f) == brute_shortest_path(n, g));
  }

  return 0;
}
