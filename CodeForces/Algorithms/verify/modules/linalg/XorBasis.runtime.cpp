#include "modules/linalg/XorBasis.hpp"

#include <cassert>
#include <random>

int main() {
  XorBasis<U64> b;
  assert(b.empty());
  assert(b.insert(0b110));
  assert(b.insert(0b011));
  assert(!b.insert(0b101));
  assert(b.size() == 2);
  assert(b.contains(0b101) && !b.contains(0b111));
  assert(b.max_xor() == 6);
  assert(b.min_xor(0b111) == 1);
  assert(b.kth(0) == 0 && b.kth(1) == 3 && b.kth(2) == 5 && b.kth(3) == 6);
  assert(isz(b.to_vec()) == 2);

  // Cross-check kth / count_less / max_xor against the enumerated span.
  std::mt19937 rng(20260804);
  FOR(iter, 200) {
    XorBasis<U64> basis;
    Vec<U64> gen;
    const I32 cnt = 1 + I32(rng() % 5);
    FOR(i, cnt) {
      const U64 v = rng() % 64;
      gen.push_back(v);
      basis.insert(v);
    }

    Vec<U64> span{0};
    for (U64 v : gen) {
      Vec<U64> nxt = span;
      for (U64 s : span) nxt.push_back(s ^ v);
      UNIQUE(nxt);
      span = nxt;
    }
    assert(isz(span) == (1 << basis.size()));

    FOR(k, isz(span)) assert(basis.kth(U64(k)) == span[k]);
    FOR(x, 70) {
      U64 want = 0;
      for (U64 s : span) want += (s < U64(x));
      assert(basis.count_less(U64(x)) == want);
    }
    assert(basis.max_xor() == span.back());
    assert(basis.min_xor() == 0);
    for (U64 s : span) assert(basis.contains(s));
  }

  // merge spans the union of both bases.
  XorBasis<U32> x, y;
  x.insert(1);
  y.insert(2);
  const auto merged = XorBasis<U32>::merge(x, y);
  assert(merged.size() == 2 && merged.contains(3));
  return 0;
}
