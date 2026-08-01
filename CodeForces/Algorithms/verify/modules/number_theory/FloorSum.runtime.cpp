#include "modules/number_theory/FloorSum.hpp"

#include <cassert>
#include <random>

namespace {

/// @brief Reference implementation, with floor division for negative numerators.
auto naive_floor_sum(I64 n, I64 m, I64 a, I64 b) -> I64 {
  I64 acc = 0;
  FOR(i, n) {
    const I64 x = a * i + b;
    acc += (x >= 0 ? x / m : -((-x + m - 1) / m));
  }
  return acc;
}

auto naive_mod_sum(I64 n, I64 m, I64 a, I64 b) -> I64 {
  I64 acc = 0;
  FOR(i, n) acc += ((a * i + b) % m + m) % m;
  return acc;
}

} // namespace

int main() {
  assert(floor_sum(0, 5, 3, 7) == 0);
  assert(floor_sum(1, 5, 3, 7) == 1);
  assert(floor_sum(4, 10, 6, 3) == 3);

  std::mt19937 rng(20260804);
  FOR(iter, 3000) {
    const I64 n = I64(rng() % 30);
    const I64 m = I64(1 + rng() % 20);
    const I64 a = I64(rng() % 41) - 20;
    const I64 b = I64(rng() % 41) - 20;
    assert(floor_sum(n, m, a, b) == naive_floor_sum(n, m, a, b));
    assert(mod_sum(n, m, a, b) == naive_mod_sum(n, m, a, b));
  }

  // Large arguments where the naive loop is not an option: check the identity
  // sum(a*i + b) == m * floor_sum + mod_sum instead.
  const I64 n = 1'000'000, m = 999'983, a = 987'654, b = 123'456;
  const I128 total = I128(a) * (n - 1) * n / 2 + I128(b) * n;
  assert(I128(m) * floor_sum(n, m, a, b) + mod_sum(n, m, a, b) == total);
  return 0;
}
