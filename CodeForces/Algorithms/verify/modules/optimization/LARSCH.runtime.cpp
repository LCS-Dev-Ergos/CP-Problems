#include "modules/optimization/LARSCH.hpp"

#include <cassert>
#include <random>

namespace {

/// Builds a Monge matrix by forcing every 2x2 second difference to be <= 0.
auto random_monge(I32 n, std::mt19937& rng) -> Vec2D<I64> {
  Vec2D<I64> a(n, VecI64(n, 0));
  FOR(j, n) a[0][j] = I64(rng() % 100);
  FOR(i, 1, n) a[i][0] = I64(rng() % 100);
  FOR(i, n - 1) {
    FOR(j, n - 1) {
      a[i + 1][j + 1] = a[i][j + 1] + a[i + 1][j] - a[i][j] - I64(rng() % 4);
    }
  }
  return a;
}

/// Leftmost argmin of row i over the columns LARSCH is allowed to look at.
auto brute_staircase_argmin(const Vec2D<I64>& a) -> VecI32 {
  const I32 n = isz(a);
  VecI32 res(n, 0);
  FOR(i, n) {
    FOR(j, 1, i + 1) {
      if (a[i][j] < a[i][res[i]]) res[i] = j;
    }
  }
  return res;
}

}  // namespace

int main() {
  std::mt19937 rng(20260809);
  FOR(iter, 300) {
    const I32 n = 1 + I32(rng() % 24);
    const Vec2D<I64> a = random_monge(n, rng);
    // Above the diagonal the matrix is unreachable, which is the staircase
    // shape LARSCH is defined on.
    auto entry = [&](I32 i, I32 j) -> I64 {
      return (j <= i ? a[i][j] : infinity<I64>);
    };

    const VecI32 expected = brute_staircase_argmin(a);
    FOR(i, n - 1) assert(expected[i] <= expected[i + 1]);

    LARSCH<I64, decltype(entry)> larsch(n, entry);
    FOR(i, n) assert(larsch.get_argmin() == expected[i]);

    // Replaying has to reproduce the same sequence.
    larsch.reset();
    FOR(i, n) assert(larsch.get_argmin() == expected[i]);
  }

  // Larger instance, so the recursion builds several reduction levels.
  {
    const I32 n = 500;
    const Vec2D<I64> a = random_monge(n, rng);
    auto entry = [&](I32 i, I32 j) -> I64 {
      return (j <= i ? a[i][j] : infinity<I64>);
    };
    const VecI32 expected = brute_staircase_argmin(a);
    LARSCH<I64, decltype(entry)> larsch(n, entry);
    FOR(i, n) assert(larsch.get_argmin() == expected[i]);
  }

  return 0;
}
