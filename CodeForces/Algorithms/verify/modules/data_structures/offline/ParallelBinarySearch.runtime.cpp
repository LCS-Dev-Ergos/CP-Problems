#include "modules/data_structures/offline/ParallelBinarySearch.hpp"

#include "modules/data_structures/union_find/DSURollback.hpp"

#include <cassert>
#include <random>

namespace {

/// Reference answer: replay the edges one by one for a single query.
auto brute_connect_time(I32 n, const Vec<PairI32>& edges, I32 s, I32 t) -> I32 {
  DSURollback dsu(n);
  FOR(i, isz(edges)) {
    dsu.unite(edges[i].first, edges[i].second);
    if (dsu.connected(s, t)) return i + 1;
  }
  return isz(edges);
}

}  // namespace

int main() {
  std::mt19937 rng(20260809);

  // Increasing predicate: the first prefix of edges that connects s and t.
  FOR(iter, 200) {
    const I32 n = 2 + I32(rng() % 12);
    const I32 m = 1 + I32(rng() % 20);
    Vec<PairI32> edges(m);
    FOR(i, m) {
      edges[i] = {I32(rng() % U32(n)), I32(rng() % U32(n))};
    }

    const I32 q = 1 + I32(rng() % 15);
    Vec<PairI32> queries(q);
    FOR(i, q) {
      const I32 s = I32(rng() % U32(n));
      I32 t = I32(rng() % U32(n - 1));
      if (t >= s) ++t;  // Keep the endpoints distinct.
      queries[i] = {s, t};
    }

    DSURollback dsu(n);
    const VecI32 got = parallel_binary_search(
        q,
        m,
        0,
        [&] { dsu.init(n); },
        [&](I32 time) { dsu.unite(edges[time].first, edges[time].second); },
        [&](I32 qi) { return dsu.connected(queries[qi].first, queries[qi].second); });

    assert(isz(got) == q);
    FOR(i, q) {
      assert(got[i] == brute_connect_time(n, edges, queries[i].first, queries[i].second));
    }
  }

  // Decreasing predicate: the last moment at which more than K components are
  // left. Searching with ok below ng exercises the reversed direction.
  FOR(iter, 100) {
    const I32 n = 3 + I32(rng() % 12);
    const I32 m = 1 + I32(rng() % 20);
    Vec<PairI32> edges(m);
    FOR(i, m) {
      edges[i] = {I32(rng() % U32(n)), I32(rng() % U32(n))};
    }

    // Stay below n so that the predicate really does hold at time 0, which the
    // routine takes on trust rather than probing.
    VecI32 threshold(1 + I32(rng() % 4));
    FOR(i, isz(threshold)) threshold[i] = 1 + I32(rng() % U32(n - 1));

    DSURollback dsu(n);
    const VecI32 got = parallel_binary_search(
        isz(threshold),
        0,
        m,
        [&] { dsu.init(n); },
        [&](I32 time) { dsu.unite(edges[time].first, edges[time].second); },
        [&](I32 qi) { return dsu.num_components() > threshold[qi]; });

    // Reference: components after each prefix, scanned backwards.
    VecI32 components(m + 1);
    DSURollback ref(n);
    components[0] = ref.num_components();
    FOR(i, m) {
      ref.unite(edges[i].first, edges[i].second);
      components[i + 1] = ref.num_components();
    }
    // Neither end of the range is probed, so the answer lives in [0, m - 1].
    FOR(i, isz(threshold)) {
      I32 want = 0;
      FOR_R(time, 1, m) {
        if (components[time] > threshold[i]) {
          want = time;
          break;
        }
      }
      assert(got[i] == want);
    }
  }

  // No queries at all, and a range with nothing to search.
  {
    I32 calls = 0;
    const VecI32 none = parallel_binary_search(
        0, 5, 0, [&] { ++calls; }, [&](I32) { ++calls; }, [&](I32) { return true; });
    assert(none.empty());
    assert(calls == 0);

    const VecI32 flat = parallel_binary_search(
        3, 1, 0, [&] { ++calls; }, [&](I32) { ++calls; }, [&](I32) { return true; });
    assert(isz(flat) == 3);
    FOR(i, 3) assert(flat[i] == 1);
    assert(calls == 0);
  }

  // Scale: on a path, edge i joins i and i+1, so s < t become connected exactly
  // after t updates.
  {
    const I32 n = 100000;
    const I32 m = n - 1;
    const I32 q = 100000;
    VecI32 qs(q);
    VecI32 qt(q);
    FOR(i, q) {
      const I32 a = I32(rng() % U32(n));
      I32 b = I32(rng() % U32(n - 1));
      if (b >= a) ++b;
      qs[i] = std::min(a, b);
      qt[i] = std::max(a, b);
    }

    DSURollback dsu(n);
    const VecI32 got = parallel_binary_search(
        q,
        m,
        0,
        [&] { dsu.init(n); },
        [&](I32 time) { dsu.unite(time, time + 1); },
        [&](I32 qi) { return dsu.connected(qs[qi], qt[qi]); });

    FOR(i, q) assert(got[i] == qt[i]);
  }

  return 0;
}
