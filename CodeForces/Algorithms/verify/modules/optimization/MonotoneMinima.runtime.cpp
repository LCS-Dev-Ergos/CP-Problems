#include "modules/optimization/MonotoneMinima.hpp"

#include <cassert>
#include <random>

namespace {

/// Builds a Monge matrix by forcing every 2x2 second difference to be <= 0.
auto random_monge(I32 h, I32 w, std::mt19937& rng) -> Vec2D<I64> {
  Vec2D<I64> a(h, VecI64(w, 0));
  FOR(j, w) a[0][j] = I64(rng() % 100);
  FOR(i, 1, h) a[i][0] = I64(rng() % 100);
  FOR(i, h - 1) {
    FOR(j, w - 1) {
      a[i + 1][j + 1] = a[i][j + 1] + a[i + 1][j] - a[i][j] - I64(rng() % 5);
    }
  }
  return a;
}

auto brute_argmin(const Vec2D<I64>& a) -> VecI32 {
  const I32 h = isz(a);
  const I32 w = isz(a[0]);
  VecI32 res(h, 0);
  FOR(i, h) {
    FOR(j, 1, w) {
      if (a[i][j] < a[i][res[i]]) res[i] = j;
    }
  }
  return res;
}

}  // namespace

int main() {
  // Degenerate shapes.
  {
    auto never = [](I32, I32, I32) -> bool { return false; };
    assert(monotone_minima(0, 5, never).empty());
    assert(smawk(0, 5, never).empty());
    const VecI32 single = {0};
    assert(monotone_minima(1, 1, never) == single);
    assert(smawk(1, 1, never) == single);
  }

  std::mt19937 rng(20260809);
  FOR(iter, 400) {
    const I32 h = 1 + I32(rng() % 14);
    const I32 w = 1 + I32(rng() % 14);
    const Vec2D<I64> a = random_monge(h, w, rng);
    auto select = [&](I32 i, I32 j, I32 k) -> bool { return a[i][k] < a[i][j]; };

    const VecI32 expected = brute_argmin(a);
    assert(monotone_minima(h, w, select) == expected);
    assert(smawk(h, w, select) == expected);

    // The leftmost argmin of a Monge matrix has to be non-decreasing.
    FOR(i, h - 1) assert(expected[i] <= expected[i + 1]);
  }

  // Larger instance, to exercise the recursion depth of both routines.
  {
    const I32 h = 300;
    const I32 w = 250;
    const Vec2D<I64> a = random_monge(h, w, rng);
    auto select = [&](I32 i, I32 j, I32 k) -> bool { return a[i][k] < a[i][j]; };
    const VecI32 expected = brute_argmin(a);
    assert(monotone_minima(h, w, select) == expected);
    assert(smawk(h, w, select) == expected);
  }

  return 0;
}
