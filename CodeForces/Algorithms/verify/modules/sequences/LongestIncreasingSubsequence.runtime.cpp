#include "modules/sequences/LongestIncreasingSubsequence.hpp"

#include <cassert>
#include <random>

namespace {

template <bool Strict>
auto brute(const VecI32& a) -> I32 {
  I32 best = 0;
  FOR(mask, 1 << isz(a)) {
    VecI32 cur;
    FOR(i, isz(a)) {
      if (mask >> i & 1) cur.push_back(a[i]);
    }
    bool ok = true;
    FOR(i, 1, isz(cur)) {
      if constexpr (Strict) ok &= cur[i - 1] < cur[i];
      else ok &= cur[i - 1] <= cur[i];
    }
    if (ok) best = std::max(best, isz(cur));
  }
  return best;
}

template <bool Strict>
void check(const VecI32& a) {
  const VecI32 idx = longest_increasing_subsequence_indices<Strict>(a);
  assert(isz(idx) == longest_increasing_subsequence_length<Strict>(a));
  assert(isz(idx) == brute<Strict>(a));
  FOR(i, isz(idx)) {
    assert(0 <= idx[i] && idx[i] < isz(a));
    if (i == 0) continue;
    assert(idx[i - 1] < idx[i]);
    if constexpr (Strict) assert(a[idx[i - 1]] < a[idx[i]]);
    else assert(a[idx[i - 1]] <= a[idx[i]]);
  }
}

}  // namespace

int main() {
  check<true>({});
  check<false>({});
  check<true>({4, 4, 4, 4});
  check<false>({4, 4, 4, 4});
  check<true>({5, 4, 3, 2, 1});
  check<false>({5, 4, 3, 2, 1});

  std::mt19937 rng(20260812);
  FOR(iter, 250) {
    const I32 n = I32(rng() % 13);
    VecI32 a(n);
    for (I32& x : a) x = I32(rng() % 9) - 4;
    check<true>(a);
    check<false>(a);
  }

  const VecI32 a = {1, 4, 2, 3};
  assert(longest_increasing_subsequence_length(a, std::greater<I32>{}) == 2);
  return 0;
}
