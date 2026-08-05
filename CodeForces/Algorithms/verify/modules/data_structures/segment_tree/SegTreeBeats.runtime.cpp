#include "modules/data_structures/segment_tree/SegTreeBeats.hpp"

#include "templates/core/MinMax.hpp"

#include <cassert>
#include <random>

namespace {

/// Reference aggregate over a plain array.
auto brute_query(const VecI64& a, I32 l, I32 r) -> Tuple<I64, I64, I64> {
  I64 sum = 0;
  I64 lo = infinity<I64>;
  I64 hi = neg_infinity<I64>;
  FOR(i, l, r) {
    sum += a[i];
    chmin(lo, a[i]);
    chmax(hi, a[i]);
  }
  return {sum, lo, hi};
}

}  // namespace

int main() {
  std::mt19937 rng(20260809);

  // Small randomized stress against a plain array, every operation checked.
  FOR(iter, 200) {
    const I32 n = 1 + I32(rng() % 40);
    VecI64 a(n);
    FOR(i, n) a[i] = I64(rng() % 41) - 20;
    BeatsSumMinMax<I64> seg(a);
    assert(seg.size() == n);

    FOR(step, 120) {
      const I32 x0 = I32(rng() % U32(n + 1));
      const I32 x1 = I32(rng() % U32(n + 1));
      const I32 l = std::min(x0, x1);
      const I32 r = std::max(x0, x1);
      const I64 v = I64(rng() % 41) - 20;

      switch (rng() % 5) {
        case 0:
          seg.add(l, r, v);
          FOR(i, l, r) a[i] += v;
          break;
        case 1:
          seg.chmin(l, r, v);
          FOR(i, l, r) chmin(a[i], v);
          break;
        case 2:
          seg.chmax(l, r, v);
          FOR(i, l, r) chmax(a[i], v);
          break;
        case 3: {
          const I32 p = I32(rng() % U32(n));
          seg.set(p, v);
          a[p] = v;
          break;
        }
        default:
          break;
      }

      const auto [sum, lo, hi] = seg.query(l, r);
      const auto [bsum, blo, bhi] = brute_query(a, l, r);
      assert(sum == bsum);
      assert(lo == blo);
      assert(hi == bhi);
    }

    assert(seg.get_all() == a);
    FOR(i, n) assert(seg.get(i) == a[i]);
    assert(seg.sum(0, n) == std::get<0>(brute_query(a, 0, n)));
    assert(seg.min(0, n) == std::get<1>(brute_query(a, 0, n)));
    assert(seg.max(0, n) == std::get<2>(brute_query(a, 0, n)));
  }

  // Clamping into a single value and back out again, which is the branch where
  // the node collapses onto one distinct element.
  {
    VecI64 a = {5, -3, 7, 0, 2, 9, -8, 4};
    BeatsSumMinMax<I64> seg(a);
    seg.chmin(0, 8, 1);
    FOR(i, 8) chmin(a[i], 1);
    seg.chmax(0, 8, 1);
    FOR(i, 8) chmax(a[i], 1);
    assert(seg.get_all() == a);
    const auto [sum, lo, hi] = seg.query(0, 8);
    assert(sum == 8 && lo == 1 && hi == 1);
    seg.add(2, 6, 10);
    FOR(i, 2, 6) a[i] += 10;
    assert(seg.get_all() == a);
  }

  // Empty ranges answer with the monoid identity.
  {
    const VecI64 a = {1, 2, 3};
    BeatsSumMinMax<I64> seg(a);
    const auto [sum, lo, hi] = seg.query(1, 1);
    assert(sum == 0);
    assert(lo == infinity<I64>);
    assert(hi == neg_infinity<I64>);
    seg.add(2, 2, 100);
    seg.chmin(0, 0, -100);
    assert(seg.get_all() == a);
  }

  // Larger array with short ranges: the whole array is still cross-checked.
  {
    const I32 n = 30000;
    VecI64 a(n);
    FOR(i, n) a[i] = I64(rng() % 2001) - 1000;
    BeatsSumMinMax<I64> seg(a);

    FOR(step, 30000) {
      const I32 l = I32(rng() % U32(n));
      const I32 r = std::min(n, l + 1 + I32(rng() % 64));
      const I64 v = I64(rng() % 2001) - 1000;
      switch (rng() % 3) {
        case 0:
          seg.add(l, r, v);
          FOR(i, l, r) a[i] += v;
          break;
        case 1:
          seg.chmin(l, r, v);
          FOR(i, l, r) chmin(a[i], v);
          break;
        default:
          seg.chmax(l, r, v);
          FOR(i, l, r) chmax(a[i], v);
          break;
      }
    }
    assert(seg.get_all() == a);
  }

  // Throughput case over full-width ranges, verified by internal consistency:
  // the root aggregate has to agree with the materialized leaves.
  {
    const I32 n = 100000;
    BeatsSumMinMax<I64> seg(n, [&](I32) -> I64 { return I64(rng() % 1000001) - 500000; });

    FOR(step, 200000) {
      const I32 x0 = I32(rng() % U32(n + 1));
      const I32 x1 = I32(rng() % U32(n + 1));
      const I32 l = std::min(x0, x1);
      const I32 r = std::max(x0, x1);
      const I64 v = I64(rng() % 1000001) - 500000;
      switch (rng() % 3) {
        case 0: seg.add(l, r, v); break;
        case 1: seg.chmin(l, r, v); break;
        default: seg.chmax(l, r, v); break;
      }
    }

    const auto [sum, lo, hi] = seg.query(0, n);
    const auto root = seg.seg.query_all();
    assert(root.sum == sum && root.min == lo && root.max == hi);
    assert(!root.fail);

    const VecI64 leaves = seg.get_all();
    assert(isz(leaves) == n);
    const auto [bsum, blo, bhi] = brute_query(leaves, 0, n);
    assert(sum == bsum && lo == blo && hi == bhi);
  }

  return 0;
}
