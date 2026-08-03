#include "modules/optimization/MinPlusConvolution.hpp"

#include <cassert>
#include <random>

namespace {

/// Convex sequence, optionally padded with infinity at either end.
auto random_convex(I32 n, std::mt19937& rng, bool pad) -> VecI64 {
  VecI64 v(n);
  I64 slope = -I64(rng() % 20);
  v[0] = I64(rng() % 50);
  FOR(i, 1, n) {
    v[i] = v[i - 1] + slope;
    slope += I64(rng() % 5);
  }
  if (pad) {
    const I32 lo = I32(rng() % (n / 2 + 1));
    const I32 hi = n - 1 - I32(rng() % (n / 2 + 1));
    FOR(i, n) {
      if (i < lo || i > hi) v[i] = infinity<I64>;
    }
  }
  return v;
}

auto random_arbitrary(I32 n, std::mt19937& rng) -> VecI64 {
  VecI64 v(n);
  FOR(i, n) {
    v[i] = (rng() % 7 == 0 ? infinity<I64> : I64(rng() % 200) - 100);
  }
  return v;
}

auto negated(const VecI64& v) -> VecI64 {
  VecI64 res(isz(v));
  FOR(i, isz(v)) res[i] = (v[i] >= infinity<I64> ? neg_infinity<I64> : -v[i]);
  return res;
}

auto brute_minplus(const VecI64& a, const VecI64& b) -> VecI64 {
  if (a.empty() || b.empty()) return {};
  VecI64 c(isz(a) + isz(b) - 1, infinity<I64>);
  FOR(i, isz(a)) {
    FOR(j, isz(b)) {
      if (a[i] >= infinity<I64> || b[j] >= infinity<I64>) continue;
      chmin(c[i + j], a[i] + b[j]);
    }
  }
  return c;
}

auto brute_maxplus(const VecI64& a, const VecI64& b) -> VecI64 {
  if (a.empty() || b.empty()) return {};
  VecI64 c(isz(a) + isz(b) - 1, neg_infinity<I64>);
  FOR(i, isz(a)) {
    FOR(j, isz(b)) {
      if (a[i] <= neg_infinity<I64> || b[j] <= neg_infinity<I64>) continue;
      chmax(c[i + j], a[i] + b[j]);
    }
  }
  return c;
}

}  // namespace

int main() {
  // Empty operands convolve to nothing.
  {
    const VecI64 empty;
    const VecI64 one = {7};
    assert(minplus_convolution_convex_convex(empty, one).empty());
    assert(minplus_convolution_arbitrary_convex(one, empty).empty());
    assert(maxplus_convolution_concave_concave(empty, empty).empty());
  }

  std::mt19937 rng(20260809);
  FOR(iter, 400) {
    const I32 n = 1 + I32(rng() % 20);
    const I32 m = 1 + I32(rng() % 20);
    const bool pad = (rng() % 2 == 0);
    const VecI64 a = random_convex(n, rng, pad);
    const VecI64 b = random_convex(m, rng, pad);

    assert(minplus_convolution_convex_convex(a, b) == brute_minplus(a, b));
    assert(minplus_convolution_arbitrary_convex(a, b) == brute_minplus(a, b));

    const VecI64 na = negated(a);
    const VecI64 nb = negated(b);
    assert(maxplus_convolution_concave_concave(na, nb) == brute_maxplus(na, nb));
    assert(maxplus_convolution_arbitrary_concave(na, nb) == brute_maxplus(na, nb));
  }

  // Only the right operand is convex, which is the common knapsack shape.
  FOR(iter, 400) {
    const I32 n = 1 + I32(rng() % 25);
    const I32 m = 1 + I32(rng() % 25);
    const VecI64 a = random_arbitrary(n, rng);
    const VecI64 b = random_convex(m, rng, rng() % 2 == 0);
    assert(minplus_convolution_arbitrary_convex(a, b) == brute_minplus(a, b));

    const VecI64 na = negated(a);
    const VecI64 nb = negated(b);
    assert(maxplus_convolution_arbitrary_concave(na, nb) == brute_maxplus(na, nb));
  }

  // Larger instance against the quadratic reference.
  {
    const VecI64 a = random_arbitrary(600, rng);
    const VecI64 b = random_convex(600, rng, true);
    assert(minplus_convolution_arbitrary_convex(a, b) == brute_minplus(a, b));
  }

  return 0;
}
