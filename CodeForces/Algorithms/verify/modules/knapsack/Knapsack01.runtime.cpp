#include "modules/knapsack/Knapsack01.hpp"

#include <cassert>
#include <random>

namespace {

auto brute(const VecI32& weight, const VecI64& value, I32 cap) -> I64 {
  I64 ans = 0;
  FOR(mask, 1 << isz(weight)) {
    I32 w = 0;
    I64 v = 0;
    FOR(i, isz(weight)) {
      if (mask >> i & 1) {
        w += weight[i];
        v += value[i];
      }
    }
    if (w <= cap) ans = std::max(ans, v);
  }
  return ans;
}

}  // namespace

int main() {
  assert(knapsack_01({}, {}, 0) == 0);
  assert(knapsack_01({0, 0, 3}, {5, 7, 10}, 0) == 12);

  std::mt19937 rng(20260812);
  FOR(iter, 400) {
    const I32 n = I32(rng() % 16);
    const I32 cap = I32(rng() % 45);
    VecI32 weight(n);
    VecI64 value(n);
    FOR(i, n) {
      weight[i] = I32(rng() % 12);
      value[i] = I64(rng() % 20);
    }
    const I64 expected = brute(weight, value, cap);
    assert(knapsack_01_by_weight(weight, value, cap) == expected);
    assert(knapsack_01_by_value(weight, value, cap) == expected);
    assert(knapsack_01_meet_in_middle(weight, value, cap) == expected);
    assert(knapsack_01(weight, value, cap) == expected);
  }

  const VecI32 weight(70, 1000);
  const VecI64 value(70, 1);
  assert(knapsack_01(weight, value, 3000) == 3);
  return 0;
}
