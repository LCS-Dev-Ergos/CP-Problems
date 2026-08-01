#include "modules/bitwise/SubsetTransform.hpp"

#include <cassert>
#include <random>

int main() {
  std::mt19937 rng(20260804);
  FOR(bits, 1, 8) {
    const I32 n = 1 << bits;
    FOR(iter, 15) {
      Vec<I64> a(n), b(n);
      FOR(i, n) {
        a[i] = I64(rng() % 21) - 10;
        b[i] = I64(rng() % 21) - 10;
      }

      Vec<I64> sub = a;
      subset_zeta(sub);
      FOR(s, n) {
        I64 want = 0;
        FOR(t, n) {
          if ((t & s) == t) want += a[t];
        }
        assert(sub[s] == want);
      }
      subset_mobius(sub);
      assert(sub == a);

      Vec<I64> sup = a;
      superset_zeta(sup);
      FOR(s, n) {
        I64 want = 0;
        FOR(t, n) {
          if ((t & s) == s) want += a[t];
        }
        assert(sup[s] == want);
      }
      superset_mobius(sup);
      assert(sup == a);

      Vec<I64> want_or(n, 0), want_and(n, 0), want_sub(n, 0);
      FOR(x, n) FOR(y, n) {
        want_or[x | y] += a[x] * b[y];
        want_and[x & y] += a[x] * b[y];
        if ((x & y) == 0) want_sub[x | y] += a[x] * b[y];
      }
      assert(or_convolution(a, b) == want_or);
      assert(and_convolution(a, b) == want_and);
      assert(subset_convolution(a, b) == want_sub);
    }
  }

  // Degenerate single-element case.
  Vec<I64> one{7};
  subset_zeta(one);
  assert(one[0] == 7);
  assert(subset_convolution(Vec<I64>{3}, Vec<I64>{4}) == Vec<I64>{12});
  return 0;
}
