#include "modules/optimization/KnuthOptimization.hpp"

#include <cassert>
#include <random>

namespace {

/// Cubic reference for the same recurrence.
template <typename F>
auto brute_interval_dp(I32 n, F w) -> Vec2D<I64> {
  Vec2D<I64> dp(n, VecI64(n, 0));
  FOR(width, 2, n + 1) {
    FOR(i, n - width + 1) {
      const I32 j = i + width - 1;
      I64 best = infinity<I64>;
      FOR(k, i, j) chmin(best, dp[i][k] + dp[k + 1][j]);
      dp[i][j] = best + w(i, j);
    }
  }
  return dp;
}

}  // namespace

int main() {
  // Degenerate sizes.
  {
    auto zero = [](I32, I32) -> I64 { return 0; };
    assert(knuth_interval_dp<I64>(0, zero).empty());
    const Vec2D<I64> one = knuth_interval_dp<I64>(1, zero);
    assert(isz(one) == 1 && one[0][0] == 0);
  }

  std::mt19937 rng(20260809);
  FOR(iter, 200) {
    const I32 n = 1 + I32(rng() % 30);
    VecI64 pref(n + 1, 0);
    FOR(i, n) pref[i + 1] = pref[i] + I64(rng() % 100);
    // A range sum satisfies the quadrangle inequality with equality, and is
    // monotone on nested intervals as long as the weights stay non-negative.
    auto w = [&](I32 i, I32 j) -> I64 { return pref[j + 1] - pref[i]; };

    assert(knuth_interval_dp<I64>(n, w) == brute_interval_dp(n, w));
  }

  // Optimal binary search tree shape, where the split point genuinely moves.
  FOR(iter, 50) {
    const I32 n = 1 + I32(rng() % 40);
    VecI64 pref(n + 1, 0);
    FOR(i, n) pref[i + 1] = pref[i] + 1 + I64(rng() % 1000);
    auto w = [&](I32 i, I32 j) -> I64 { return pref[j + 1] - pref[i]; };
    assert(knuth_interval_dp<I64>(n, w) == brute_interval_dp(n, w));
  }

  // Larger instance, still affordable for the cubic reference.
  {
    const I32 n = 160;
    VecI64 pref(n + 1, 0);
    FOR(i, n) pref[i + 1] = pref[i] + I64(rng() % 10000);
    auto w = [&](I32 i, I32 j) -> I64 { return pref[j + 1] - pref[i]; };
    assert(knuth_interval_dp<I64>(n, w) == brute_interval_dp(n, w));
  }

  return 0;
}
