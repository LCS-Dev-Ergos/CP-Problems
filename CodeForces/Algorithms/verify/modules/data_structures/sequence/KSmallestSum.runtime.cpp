#include "modules/data_structures/sequence/KSmallestSum.hpp"

#include <cassert>
#include <random>

namespace {

template <typename Compare>
auto brute(const MultiSet<I32>& values, I32 k, Compare comp) -> I64 {
  VecI32 a(all(values));
  std::sort(all(a), comp);
  I64 ans = 0;
  FOR(i, std::min(k, isz(a))) ans += a[i];
  return ans;
}

}  // namespace

int main() {
  KSmallestSum<I32, I64> small(0);
  KLargestSum<I32, I64> large(0);
  MultiSet<I32> values;
  std::mt19937 rng(20260812);

  FOR(iter, 4000) {
    if (values.empty() || rng() % 3 != 0) {
      const I32 x = I32(rng() % 101) - 50;
      values.insert(x);
      small.insert(x);
      large.insert(x);
    } else {
      auto it = values.begin();
      std::advance(it, rng() % values.size());
      const I32 x = *it;
      values.erase(it);
      small.erase(x);
      large.erase(x);
    }
    const I32 k = I32(rng() % (values.size() + 3));
    small.set_k(k);
    large.set_k(k);
    assert(small.size() == isz(values));
    assert(large.size() == isz(values));
    assert(small.sum() == brute(values, k, std::less<I32>{}));
    assert(large.sum() == brute(values, k, std::greater<I32>{}));
  }
  return 0;
}
